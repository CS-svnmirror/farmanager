#pragma once

/*
sqlitedb.hpp

������ sqlite api ��� c++.
*/
/*
Copyright � 2011 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

namespace sqlite
{
	struct sqlite3;
	struct sqlite3_stmt;
}

class SQLiteDb: NonCopyable
{
public:
	SQLiteDb();
	virtual ~SQLiteDb();

	enum ColumnType
	{
		TYPE_INTEGER,
		TYPE_STRING,
		TYPE_BLOB,
		TYPE_UNKNOWN
	};

	bool IsNew() const { return db_exists <= 0; }
	int InitStatus(string& name, bool full_name);

protected:
	class SQLiteStmt: NonCopyable
	{
	public:
		SQLiteStmt(){};
		SQLiteStmt(sqlite::sqlite3_stmt* Stmt): m_Stmt(Stmt), m_Param(1) {}
		SQLiteStmt(SQLiteStmt&& rhs) { *this = std::move(rhs); };

		MOVE_OPERATOR_BY_SWAP(SQLiteStmt);

		void swap(SQLiteStmt& rhs) noexcept
		{
			std::swap(m_Param, rhs.m_Param);
			std::swap(m_Stmt, rhs.m_Stmt);
		}

		struct blob
		{
			blob(const void* Data, size_t Size): m_Data(Data), m_Size(Size) {}
			const void* const m_Data;
			const size_t m_Size;
		};

		template<class T>
		struct transient_t
		{
			transient_t(const T& Value): m_Value(Value) {}
			const T& m_Value;
		};

		bool Finalize() const;
		SQLiteStmt& Reset();
		bool Step() const;
		bool StepAndReset();

		template<typename T>
		SQLiteStmt& Bind(T&& Arg) { return BindImpl(std::forward<T>(Arg)); }

#if defined _MSC_VER && _MSC_VER < 1800
		#define BIND_VTE(TYPENAME_LIST, ARG_LIST, REF_ARG_LIST, FWD_ARG_LIST) \
		template<VTE_TYPENAME(first), TYPENAME_LIST> \
		SQLiteStmt& Bind(VTE_REF_ARG(first), REF_ARG_LIST) \
		{ \
			return Bind(VTE_FWD_ARG(first)), Bind(FWD_ARG_LIST); \
		}

		#include "common/variadic_emulation_helpers_begin.hpp"
		VTE_GENERATE(BIND_VTE)
		#include "common/variadic_emulation_helpers_end.hpp"

		#undef BIND_VTE
#else
		template<typename T, class... Args>
		SQLiteStmt& Bind(T&& arg, Args&&... args)
		{
			return Bind(std::forward<T>(arg)), Bind(std::forward<Args>(args)...);
		}
#endif

		const wchar_t *GetColText(int Col) const;
		const char *GetColTextUTF8(int Col) const;
		int GetColBytes(int Col) const;
		int GetColInt(int Col) const;
		unsigned __int64 GetColInt64(int Col) const;
		const char *GetColBlob(int Col) const;
		ColumnType GetColType(int Col) const;

	private:
		SQLiteStmt& BindImpl(int Value);
		SQLiteStmt& BindImpl(__int64 Value);
		SQLiteStmt& BindImpl(const string& Value, bool bStatic = true);
		SQLiteStmt& BindImpl(const blob& Value, bool bStatic = true);
		SQLiteStmt& BindImpl(unsigned int Value) { return BindImpl(static_cast<int>(Value)); }
		SQLiteStmt& BindImpl(unsigned __int64 Value) { return BindImpl(static_cast<__int64>(Value)); }
		template<class T>
		SQLiteStmt& BindImpl(const transient_t<T>& Value) { return BindImpl(Value.m_Value, false); }

		struct stmt_deleter { void operator()(sqlite::sqlite3_stmt*) const; };
		std::unique_ptr<sqlite::sqlite3_stmt, stmt_deleter> m_Stmt;
		int m_Param;
	};

	typedef SQLiteStmt::blob blob;
	template<class T>
	static SQLiteStmt::transient_t<T> transient(const T& Value) { return SQLiteStmt::transient_t<T>(Value); }

	bool Open(const string& DbName, bool Local, bool WAL=false);
	void Close();
	void Initialize(const string& DbName, bool Local = false);
	bool InitStmt(SQLiteStmt &stmtStmt, const wchar_t *Stmt) const;
	template<size_t N>
	bool PrepareStatements(const simple_pair<int, const wchar_t*>(&Init)[N]) { return PrepareStatements(Init, N); }
	bool Exec(const char *Command) const;
	bool SetWALJournalingMode() const;
	bool EnableForeignKeysConstraints() const;
	int Changes() const;
	unsigned __int64 LastInsertRowID() const;

	bool BeginTransaction() const;
	bool EndTransaction() const;
	bool RollbackTransaction() const;

	string strPath;
	string m_Name;

	std::vector<SQLiteStmt> m_Statements;

private:
	virtual bool InitializeImpl(const string& DbName, bool Local) = 0;
	bool PrepareStatements(const simple_pair<int, const wchar_t*>* Init, size_t Size);

	struct db_closer { void operator()(sqlite::sqlite3*) const; };
	typedef std::unique_ptr<sqlite::sqlite3, db_closer> database_ptr;

	database_ptr m_Db;
	int init_status;
	int db_exists;
};
