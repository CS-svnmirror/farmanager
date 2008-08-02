#ifndef __TLIST_HPP__
#define __TLIST_HPP__
/*  TList.hpp
    ������� ������ ������ � ���������� ������� / by Spinoza /
    Object ������ ����� ����������� �� ��������� � ��������� ���������:
      const Object& operator=(const Object &)
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

template <class Object>
class TList
{
  private:
    struct OneItem
    {
      Object  Item;
      OneItem *Prev, *Next;
    }
      *First, *Last, *Current, *Tmp, *SavedPos;

    DWORD Size;

  public:
    TList(): First(NULL), Last(NULL), Current(NULL), SavedPos(0), Size(0)
      {}
    ~TList() { clear(); }

  public:
    // �������� ���������� ��������� � ������
    DWORD size() const { return Size; }

    // ���������� TRUE, ���� ������ ����
    BOOL empty() const { return !Size; }

    //  �������� ��������� �� ������� ������� ������ (���������� NULL ��� �������)
    const Object *getItem() { return Current?&Current->Item:NULL; }

    // ���������� TRUE, ���� ������� ������� ����������� �� ������ �������
    BOOL isBegin() const { return Current==First; }

    // ���������� TRUE, ���� ������� ������� ����������� �� ��������� �������
    BOOL isEnd() const { return Current==Last; }

    // ���� � ������ ������ � ������� ��������� �� ������ �������
    // ������������ NULL, ���� ������ ����
    const Object *toBegin()
    {
      Current=First;
      return Current?&Current->Item:NULL;
    }

    // ���� � ����� ������ � ������� ��������� �� ��������� �������
    // ������������ NULL, ���� ������ ����
    const Object *toEnd()
    {
      Current=Last;
      return Current?&Current->Item:NULL;
    }

    // ���� � ���������� �������� ������ � ������� ��������� �� ����
    // (���������� NULL, ���� ��������� ����� ������ ��� ������� �������
    // �� ��� ���������)
    const Object *toNext()
    {
      return (Current && NULL!=(Current=Current->Next))?&Current->Item:NULL;
    }

    // ���� � ����������� �������� ������ � ������� ��������� �� ����
    // (���������� NULL, ���� ���������� ������ ������ ��� ������� �������
    // �� ��� ���������)
    const Object *toPrev()
    {
      return (Current && NULL!=(Current=Current->Prev))?&Current->Item:NULL;
    }

    // �������� ������� � ������ ������
    // ������� ������� ��� ������ ��������������� �� ���� �������
    // ��� ������� ������������ FALSE
    BOOL push_front(const Object &Source)
    {
      SavedPos=NULL;
      Tmp=new OneItem;
      if(Tmp) // ���������, ���� �� ����� ����������
      {
        Tmp->Item=Source;
        Tmp->Prev=NULL;
        if(First)
          First->Prev=Tmp;
        Tmp->Next=First;
        First=Current=Tmp;
        if(!Last) // ������ �� �������� ��� ����
          Last=First;
        ++Size;
        return TRUE;
      }
      return FALSE;
    }

    // �������� ������� � ����� ������
    // ������� ������� ��� ������ ��������������� �� ���� �������
    // ��� ������� ������������ FALSE
    BOOL push_back(const Object &Source)
    {
      SavedPos=NULL;
      Tmp=new OneItem;
      if(Tmp) // ���������, ���� �� ����� ����������
      {
        Tmp->Item=Source;
        if(Last)
          Last->Next=Tmp;
        Tmp->Prev=Last;
        Tmp->Next=NULL;
        Last=Current=Tmp;
        if(!First) // ������ �� �������� ��� ����
          First=Last;
        ++Size;
        return TRUE;
      }
      return FALSE;
    }

    // �������� ������� ����� ������� ������� � ������
    // ���� ������� ������� �� ����������, �� ������� ����������� � ����� ������ (=push_back)
    // ������� ������� ��� ������ ��������������� �� ����� �������
    // ��� ������� ������������ FALSE
    BOOL insert(const Object &Source)
    {
      if(!Current)
        return push_back(Source);
      SavedPos=NULL;
      Tmp=new OneItem;
      if(Tmp) // ���������, ���� �� ����� ����������
      {
        if(isEnd())
          Last=Tmp;
        Tmp->Item=Source;
        Tmp->Next=Current->Next;
        Tmp->Prev=Current;
        Current->Next=Tmp;
        Current=Tmp;
        ++Size;
        return TRUE;
      }
      return FALSE;
    }

    // ������� �������, ������� ������� ��������������� �� ��������� �������
    // ���� ������� ������� �� �������� �� ����������, �� ������������ FALSE
    BOOL removeToEnd()
    {
      SavedPos=NULL;
      if (Current)
      {
        if(isEnd())
          Last=Last->Prev;
        Tmp=Current->Next;
        delete Current;
        Current=Tmp;
        --Size;
        if(!Size)
          First=Last=NULL;
        return TRUE;
      }
      return FALSE;
    }

    // ������� removeToEnd
    BOOL erase() { return removeToEnd(); }

    // ������� �������, ������� ������� ��������������� �� ���������� �������
    // ���� ������� ������� �� �������� �� ����������, �� ������������ FALSE
    BOOL removeToBegin()
    {
      SavedPos=NULL;
      if (Current)
      {
        if(isBegin())
          First=First->Next;
        Tmp=Current->Prev;
        delete Current;
        Current=Tmp;
        --Size;
        if(!Size)
          First=Last=NULL;
        return TRUE;
      }
      return FALSE;
    }

    // �������� ������ �� ���������� ������ lst � ��������
    void swap(TList<Object> &lst)
    {
      OneItem *newFirst=lst.First, *newLast=lst.Last,
              *newCurrent=lst.Current, *newSavedPos=lst.SavedPos;
      lst.First=First;
      lst.Last=Last;
      lst.Current=Current;
      lst.SavedPos=SavedPos;
      First=newFirst;
      Last=newLast;
      Current=newCurrent;
      SavedPos=newSavedPos;
    }

    // ��������� ������� ������� (��. restorePosition) ��� ������������
    // ��������������. ������������ FALSE, ���� ������� ������� ����
    // ��������������
    BOOL storePosition()
    {
      SavedPos=Current;
      return SavedPos!=NULL;
    }

    // ������������ ������� ������� (��. storePosition) ��� ������������
    // ��������������. ������������ FALSE, ���� ������� ������� �����
    // ��������������
    BOOL restorePosition()
    {
      Current=SavedPos;
      return Current!=NULL;
    }

    // �������� ������
    void clear()
    {
      toBegin();
      while(erase())
        ;
      SavedPos=NULL;
    }


  private:
    TList& operator=(const TList& rhs); /* ����� �� �������������� */
    TList(const TList& rhs);            /* �� ���������            */
};

#endif // __TLIST_HPP__
