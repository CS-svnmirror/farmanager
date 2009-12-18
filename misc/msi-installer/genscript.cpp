#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  try {
    if (argc < 2)
      throw exception("source path not specified");

    if (argc < 3)
      throw exception("platform not specified");

    string source_dir = argv[1];

    // determine Far version
    string plugin_hpp_path = source_dir + "\\PluginSDK\\Headers.c\\plugin.hpp";
    ifstream header_file;
    header_file.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
    header_file.open(plugin_hpp_path.c_str());
    string line;
    unsigned ver_major, ver_minor, ver_build;
    unsigned fount_cnt = 0;
    while (fount_cnt < 3) {
      header_file >> line;
      if (line == "FARMANAGERVERSION_MAJOR") {
        header_file >> ver_major;
        fount_cnt++;
      }
      if (line == "FARMANAGERVERSION_MINOR") {
        header_file >> ver_minor;
        fount_cnt++;
      }
      if (line == "FARMANAGERVERSION_BUILD") {
        header_file >> ver_build;
        fount_cnt++;
      }
    }

    // determine Far platform
    string platform = argv[2];

    // generate makefile
    ostringstream fmt;
    fmt << ver_major << "." << ver_minor << "." << ver_build;
    string version = fmt.str();
    fmt.str(string());
    fmt << "Far" << ver_major << ver_minor << "b" << ver_build << "." << platform << ".msi";
    string msi_name = fmt.str();

#ifdef SPECIAL
    msi_name = source_dir + "\\final.msi";
#endif

    ofstream makefile;
    makefile.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
    makefile.open("makefile");
    makefile << "all:" << endl;
    makefile << "  cl -nologo -O1 -EHsc customact.cpp -link -dll -out:CustomActions.dll -export:UpdateFeatureState -export:SaveShortcutProps -export:RestoreShortcutProps msi.lib" << endl;
    makefile << "  -upx --lzma CustomActions.dll" << endl;
    makefile << "  candle -nologo -dSourceDir=\"" << source_dir << "\" -dBranch=" << ver_major << " -dPlatform=" << platform << " -dVersion=" << version << " installer.wxs ui.wxs" << endl;
    makefile << "  light -nologo -cultures:en-us -loc en-us.wxl -loc ui_en-us.wxl -spdb -sval -sh -dcl:high -out " << msi_name << " installer.wixobj ui.wixobj" << endl;

    return 0;
  }
  catch (const exception& e) {
    cerr << "genscript: error: " << typeid(e).name() << ": " << e.what() << endl;
  }
  catch (...) {
    cerr << "genscript: unknown error" << endl;
  }
  return 1;
}
