#include <cassert>
#include <vector>
#include <map>
#include <cstdlib>
#include <string>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>//chdir,execvp,execvpe, File descriptor
#include <fcntl.h>
#include <cstdio>//perror
#include <sys/types.h>//waitpid
#include <sys/wait.h>
#include <sys/stat.h>//open
#include <cctype>
#include <dirent.h>
 
#define WhiteSpace " \t\n\v\f\r"
//define all the white spaces in c++ reference
//http://www.cplusplus.com/reference/cctype/isspace/
 
using std::invalid_argument;

/*IsEscape() find if the character at in[loc] is escaped.
 *if so return true else false.
 *the implementation is as follow:
 *count the number of \ before ' '
*/
bool IsEscape(std::string in, size_t loc);

/*FindDelimiter() search the specifed delimter in input string in from loca-
 *tion loc. 
 *It returns the location of delimter if found.
 *or return npos if not found. 
*/
size_t FindDelimiter(std::string & in, std::string delimiter, size_t loc);

/*SetIOException: used to resort IO
 *from c++ reference
 *http://www.cplusplus.com/reference/exception/exception/exception/
*/
class SetIOException : public std::exception {
 public:
  const char * what () const throw () {
    return "SetIO Error, But defult IO resorted";
  }
};
/*MyshellClass consititutes of the following member:
 *InputStr storing one input line from stdin.
 *ISEXIT() method return the ISEXIT member indiciating if input is EOF or eixt. 
 *A command list containing all input commands.
 *CurrentWorkingDir is initialising from $PWD variable.
 *Map LocalVars is a static member shared by all MyShellClass member in a programe. 
 *char ** GlobalEnv points at the enironment variables of the main program.
*/
class MyShellClass{
private:
  /*Command calss consists of :
   *A vector string containing all arguments of this command
   *There are three files whcich are the redirect files.
  */
  class Command{
  public:
    std::vector<std::string> argvs;
    std::string InputFile;
    std::string OutputFile;
    std::string ErrFile;
    Command(){
    } 
    Command(const Command & rhs){
      *this=rhs;
    }
    Command & operator = (const Command & rhs){
      if(this != & rhs){
	argvs=rhs.argvs;
	InputFile=rhs.InputFile;
	ErrFile=rhs.ErrFile;
	OutputFile=rhs.OutputFile;
      }
      return *this;
    }
    ~Command(){
    }
    void PrintArgv();
    std::string GetCommandName();
  };
  //MyShellClass member
  bool ISEXIT;//EOF or exit
  std::string InputStr;
  std::vector<Command> CommandList;
  char ** GlobalEnv;
  std::string CurrentWorkingDir;
  static std::map <std::string,std::string> LocalVars;
  
  //functions for ReadIn
  /*ParseVar() searches character which is not alphanumeric from begin
   *the return value is the location of that char
   *If not found, returns npos 
   */
  size_t ParseVar(std::string instr, size_t begin);
  /*ReplaceVar() replace $variable in InputStr interatelly.
   *Note it also works if $variable is a  global variable 
   */
  void ReplaceVar();
  /*GetValue() outputs the value of input key(variable name)*/
  std::string GetValue(std::string key);
 /*this function split the InputStr with '|'
  *then parse the splited substring by calling ParseOneCommand()
  */
  void GetCommandList();
  /*ParseOneCommand create a new command with redirect files and
   *Push the command at the back of CommandList.
   *It first checks if the input command is all empty, then: 
   *Calling FindRedirect to find redirect files.
   *Calling ParseArgvs to find arguments.
  */
  void ParseOneCommand(std::string instr);
  /*ParseFileName find the first non-space character in Instr set as begin
   *and the first space after it which is set and end
   *the it return filename string from begin(inclusive) to end(exclusive) 
   */
  std::string ParseFileName(std::string Instr);
  /* FindRedirect first check the format of current command input
   *it then split the string into substr according to redirect symbol
   *Then calling ParseFileName to obtian the valid filename
   */
  void FindRedirect(std::string & instr, Command & curr);
  /*ParseArgvs splite the command argument with space
   *once it find an argument it Calls WriteArgs to write argv to argvs class meber.
   */
  void ParseArgvs(std::string  instr, Command & curr);
  /* writeArgs erase \ in the string in while preserve the
     char after a \.
     Then store string in cahr** newargv, increment newargc
  */
  void WriteArgs(std::string in,std:: vector<std::string> & argvs);
  //functions for RunCommand
  /*IsBuiltIn will check if the input command is 
   *exit, cd, set, or export.
  **/
  bool IsBuiltIn(Command & com);
  void ChDir(Command & com);
  void SetVar(Command & com);
  void ExportVar(Command & com);
  bool CheckDir(const char* filename, const char* dirname);
  
  /*GetPath will check if there is / in file which is the com.argvs[0].
   *note the file name does not contain \.
  */
  void GetPath(Command & com);
  /*SetIO set the file descriptor 0,1,2 to the redirect files in the command 
   *If there is a redirect output file, it return -1,
   *else it creates a pipe and return the ReadEnd pfd[0]. 
   */
  int SetIO(int ReadEnd, Command & com);
  /*ExecuteCommand a command and returns its pid. When the command is builtin command, returns -1.  
   *If a builtin comand is not in pipe, ExecuteCommand runs it in parent process 
   *Else ExecuteCommand runs it in forked child process.
   */
  pid_t ExecuteCommand(Command & com);
  /*HandleSigna set current IO 0,1,2 to the input StdinNo, StdoutNo, StderrNo seperately. And wait for the process of which pid is cpid to end.
  *Then report its status.
  */
  void HandleSignal(int StdinNo, int StdoutNo, int StderrNo,pid_t cpid);

public:
 MyShellClass(char **env):ISEXIT(false),GlobalEnv(env) {
    std::string homepath(getenv("PWD"));
    CurrentWorkingDir = homepath;
  }
  bool IsExit(){
    return ISEXIT;
  }
  size_t CommandNum(){
    return CommandList.size();
  }
  void PrintCommandList();

  /*ReadIn function read from stdin, parse the input create the CommandList
   *if it encounter EOF or exit, it set ISEXIT to true.
   *Note if mutiple redirect files are specified, ReadIn() only store the 
   *last one.
   */
  void ReadIn();
  
  /*RunCommandList() takes no extra parameters. 
   *It runs the Commands in the CommandList method of MyShell class.
   *If a builtin function-cd, set,export-is the only command myShell received
   *, the function is run in current process(parent process).
   *Otherwise, it is run in forked child process.
   *If any error happens in this process, an SetIOException is throw,
   *which is catched in RunCommandList(). 
   *Then any vaild file descriptor is closed and Default IO is resorted. 
   *Its detailed implementation as follow:
   *Calling SetIO(Command & com) to set the Std io for a command.
   *check if the returned File descriptor of pipe is valid.
   *If not, set it as default StdIn. 
   *this value is the ReadEnd of next command.
   *Calling ExecuteCommand(Command & com) to fork and execve command.
   *Calling HandleSignal to resort default IO for parent process,
   *and wait for the last command end. 
   *Then print the status of the last command,
   */
  void RunCommandList();
};
