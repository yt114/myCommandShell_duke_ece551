#include "my_shell_class.h"
bool IsEscape(std::string in, size_t loc){
  if(loc==0){//no character before loc
    return false;
  }
  size_t count_back = 0;
  while(in[loc-count_back-1]=='\\'){
    count_back++;
  }
  if(count_back%2==0){//the number of character'\\' before loc is even
    return false;
  }
  else{
    return true;
  }
}

size_t FindDelimiter(std::string & in, std::string delimiter, size_t loc){
  if(loc==std::string::npos){
    return loc;
  }
  loc=in.find_first_of(delimiter,loc);
  if(loc==std::string::npos){
    return loc;//not find delimiter return npos
  }
  else{
    if(IsEscape(in,loc)){// the delimiter is escaped
      return FindDelimiter(in,delimiter, loc+1);
    }
    else{
      return loc;
    }
  }
}
  
 
void MyShellClass::Command::PrintArgv(){
  std::cout<<"argvs size="<<argvs.size()<<std::endl;

  for (size_t i = 0;i<argvs.size(); i++){
    std::cout<<"argv"<<i<<" :"<<argvs[i]<<std::endl;
  }
  
  std::cout<<"Output file="<<OutputFile<<" Inputfile="<<InputFile<<" ErrorFile="<<ErrFile<<std::endl;
}

std::string MyShellClass::Command::GetCommandName(){  
  if(!argvs.empty()){
    return argvs[0];
  }
 else{
   throw invalid_argument("argvs is empty");
 }
}

size_t MyShellClass::ParseVar(std::string instr, size_t begin){
  for(size_t i = begin; i<instr.length();i++){
    int c = (int) instr[i];
    if(!isalnum (c)&&c!=95){//95 =_
      return i;
    }
  }
  return std::string::npos;
}
void MyShellClass::ReplaceVar(){
  size_t begin = 0;
  size_t end = begin;
  std::string delimiter("$");
  std::string DeWithSpace = WhiteSpace+delimiter;
  while((begin=FindDelimiter(InputStr,delimiter,end))!=std::string::npos){//find the delimiter
    if(begin==InputStr.length()-1){//jusde \$ at the end of string
      throw std::invalid_argument("Input command format is wrong");  
    }
    else{
      end=ParseVar(InputStr,begin+1);
      if(end!=std::string::npos){
	std::string Key = InputStr.substr(begin+1,end-begin-1);
	std::string Value = GetValue(Key);
	InputStr.replace(begin,end-begin,Value);
      }
      else{
	std::string Key = InputStr.substr(begin+1);	
	std::string Value = GetValue(Key);
	InputStr.replace(begin,end,Value);//end = npos
      }
    }
  }
}

std::string MyShellClass::GetValue(std::string key){
  std::map<std::string,std::string>::iterator it = LocalVars.find(key);//from c++ reference
  char* SystemVar =NULL; 
  if (it != LocalVars.end()){//find key in LocalVars class member
    return it->second;
  }
  else if((SystemVar = getenv(key.c_str()))!=NULL){// find in system var
    std::string Value(SystemVar);
    return Value;
  }
  else{// not find in loc var and system var    
    throw std::invalid_argument("No such Var"); 
  }
}

void MyShellClass::GetCommandList(){
  size_t begin = 0;
  size_t end = begin;
  std::string delimiter("|");
  while((end=FindDelimiter(InputStr,delimiter,begin))!=std::string::npos){
    if(end==InputStr.length()-1){
      throw invalid_argument("piple error : wrong format");
    }
    ParseOneCommand(InputStr.substr(begin,end-begin));
    begin = end+1;
  }
  ParseOneCommand(InputStr.substr(begin));
}


void MyShellClass::ParseOneCommand(std::string instr){
  std::string white(WhiteSpace);
  std::size_t begin = instr.find_first_not_of(WhiteSpace);
  if(begin==std::string::npos){//check if string are empty
    throw std::invalid_argument("Input command is empty");
  }
  else if(instr[instr.length()-1]=='\\'&&IsEscape(instr,instr.length()-1)==false){//\ in the end
    throw std::invalid_argument("wrong format");  
  }
  else{
    Command CurrCommand;
    FindRedirect(instr,CurrCommand);
    //ERASE the redirect part in instr
    std::string delimiter("<>");
    size_t FirstRe = FindDelimiter(instr,delimiter,0);
    if(FirstRe!=std::string::npos){
      if(instr[FirstRe-1]=='2'&&IsEscape(instr,FirstRe-1)==false){//2>
	if(instr[FirstRe-2]==' '){
	instr.erase(FirstRe-1);
	}
	else{
	  instr.erase(FirstRe);
	}
      }
      else{
	instr.erase(FirstRe);
      }
    } 
    //pass the instr withour redirect part.
    ParseArgvs(instr,CurrCommand);
    CommandList.push_back(CurrCommand);
  }
}

std::string MyShellClass::ParseFileName(std::string instr){
  std::string SpaceDe(WhiteSpace);
  size_t file_begin = instr.find_first_not_of(WhiteSpace);
  size_t file_end = FindDelimiter(instr,SpaceDe,file_begin+1);
  std::string file = instr.substr(file_begin,file_end-file_begin);
  //exclude the end space
  return file;
}

void MyShellClass::FindRedirect(std::string & instr, Command & curr){
  size_t begin = 0;
  size_t end = begin;
  std::string delimiter("<>");

  do{//begin marks a < >
    end=FindDelimiter(instr,delimiter,begin+1);
    if(end==instr.length()-1){
      throw std::invalid_argument("Input command format is wrong");  
    }
    if (begin!=0){
      if(end==begin+1){
	throw std::invalid_argument("Input command format is wrong"); 
      }
      std::string file = ParseFileName(instr.substr(begin+1,end-begin-1));
      if(instr[begin]=='<'){
	curr.InputFile=file;
      }
      else if(instr[begin]=='>'){
	if(instr[begin-1]=='2'&&IsEscape(instr,begin-1)==false){
	  if(begin-1==0){//command strart with 2>
	    throw std::invalid_argument("Input command format is wrong");
	  }
	  else if(instr[begin-2]!=' '){
	    curr.OutputFile=file;
	  }
	  else{
	  curr.ErrFile=file;//error file case
	  }
	}
	else{
	  curr.OutputFile=file;
	}
      }
    }
    begin = end;
  }while(end!=std::string::npos);
}
   
void MyShellClass::ParseArgvs(std::string instr,Command & curr){
  std::string delimiter(WhiteSpace);
  size_t begin = instr.find_first_not_of(delimiter);
  if(begin==std::string::npos){//the rest of string are all blank
    return;
  }
  else{
    size_t end = FindDelimiter(instr, delimiter, begin);
    // begin is non-space and end is space
    if(end!=std::string::npos){
      WriteArgs(instr.substr(begin,end-begin),curr.argvs);
      //include begin exlude end
      ParseArgvs(instr.substr(end+1),curr);
      //split the rest string from end+1
    }
    else{
      WriteArgs(instr.substr(begin),curr.argvs);  
      return;
    }
  }
}

void MyShellClass::WriteArgs(std::string in, std::vector<std::string> &argvs){
  std::string temp;
  for (size_t i=0;i<in.length();i++){//escape character
    if(in[i]=='\\'){
      i++;
    }
    temp.push_back(in[i]);
  }
  argvs.push_back(temp);
}

bool MyShellClass::IsBuiltIn(Command & com){
  if(com.argvs[0]=="exit"){
    exit(EXIT_SUCCESS);
  }
  //only forked child processes run exit command in this function frame.
  //parent process exit if and only if commandlist.size() is 1
  //meanwhile com.argvs[0] is exit.
  else if(com.argvs[0]=="cd"){
    ChDir(com);
  }
  else if(com.argvs[0]=="set"){
    SetVar(com);
  }
  else if(com.argvs[0]=="export"){
    ExportVar(com);
  }
  else{
    return false;
  }
  return true;
}

void MyShellClass::ChDir(Command & com){
   switch((com.argvs).size()){
   case 1:{
      if (chdir(getenv("HOME")) != 0){
	throw invalid_argument("cd error: colud not open $Home");
      }
      else{
	std::string env(getenv("HOME"));	
	CurrentWorkingDir = env;
      }
   }
      break;
   case 2:{
      const char * path = (com.argvs[1]).c_str();
      if (chdir(path) != 0){
	throw invalid_argument("cd error: colud not open argv[1]");
      }
      else{
	char * cwd = get_current_dir_name();
	std::string env(cwd);
	CurrentWorkingDir = env;
	free(cwd);
      }
   }
      break;
   default:
      throw invalid_argument("cd error !");
   }

   if((setenv("PWD",CurrentWorkingDir.c_str(),1))!=0){
    perror("putenv");
    throw invalid_argument("Export PWD error !");
  }
}

void MyShellClass::SetVar(Command & com){
  if((com.argvs).size()<3){//no variable name or value
    throw invalid_argument("Error: empty variable name or value");
  }
  std::string key = com.argvs[1];
  if(ParseVar(key,0)!=std::string::npos){//illegal char in var name
    throw invalid_argument("Error: illegal variable name");
  }
  std::string delimiter(WhiteSpace);
  size_t begin = InputStr.find(key)+key.length();
  begin = InputStr.find_first_not_of(delimiter,begin);
  std::string value = InputStr.substr(begin);
  LocalVars[key]=value;
}

void MyShellClass::ExportVar(Command & com){
  if(com.argvs.size()<2){
    throw invalid_argument("Error: export empty variable name");
  }
  if(com.argvs.size()>2){
    throw invalid_argument("Error: export more than one var");
  }
  std::string key=com.argvs[1];
  std::string value = GetValue(key);
  if((setenv(key.c_str(),value.c_str(),1))!=0){
    perror("putenv");
    throw invalid_argument("ExportVar error !");
  }
}

bool MyShellClass::CheckDir(const char* filename, const char* dirname){
  DIR * dir = opendir(dirname);
  if(dir==NULL){
    perror("CheckDir error!");
    exit(EXIT_FAILURE);
  }
  struct dirent * file;
  while((file=readdir(dir))!=NULL){
    if(file->d_type==DT_LNK||file->d_type==DT_REG){
      //  std::cout<<"file->d_name "<<file->d_name<<"  file->d_type "<<file->d_type<<std::endl;
      if(!strcmp(file->d_name,filename)){//check file in the dir which are regular or link
	closedir(dir);
	return true;
      }
    }
  }
  closedir(dir);
  return false;
}

void MyShellClass::GetPath(Command & com){
  std::string filepath(com.argvs[0]);
  bool IsFileInCurrPath=false;
  if(filepath.find_first_of('/')==std::string::npos){//if not contain'\'
    char * EnvList = getenv("PATH"); 
    if(EnvList==NULL){
      std::cerr<<"EnvList is null";
      exit(EXIT_FAILURE);
    }
    std::string EnvStr(EnvList);//split Envstr get Envlist
    size_t end=0;
    while(IsFileInCurrPath!=true&&!EnvStr.empty()){
      //File is not found and there are path in EnvStr to search
      std::string OnePath;
      end=EnvStr.find_first_of(':');//the end of first path
      if(end!=std::string::npos){//more than one path
	OnePath = EnvStr.substr(0,end);//include str[0] exclude str[end]
	EnvStr.erase(0,end+1);//include :
      }
      else{//the rest is a path
	OnePath = EnvStr;
	EnvStr.erase(0);//EnvStr is empty now
      }
      IsFileInCurrPath=CheckDir(filepath.c_str(), OnePath.c_str());
      if(IsFileInCurrPath==true){//if find filname append path
	com.argvs[0] = OnePath+'/'+filepath;
	return;
      }
    } 
    if(IsFileInCurrPath==false){
      std::cerr<<"Command "<<filepath<<" not found"<<std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

pid_t MyShellClass:: ExecuteCommand(Command & com){
  if(CommandList.size()==1){//The only com is buildin fun
    if(IsBuiltIn(com)){//run it in parent process
      return -1;
    }
  }//in the case of pipe, all fun are run in child process
  pid_t cpid = fork();//fork
  if(cpid == -1) {//fail to fork
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if(cpid==0){//child
    try{
      if(!IsBuiltIn(com)){//not buildin fun
	std::vector<char*> arr;
	for(std::vector<std::string>::iterator it = (com.argvs).begin(); it != (com.argvs).end(); ++it)
	  {
	    arr.push_back(&(*it)[0]);//const chat* str; 
	  }
	arr.push_back(NULL);
	GetPath(com);//check if comm has path 
	execve(arr[0], &arr[0],GlobalEnv);
	perror("execve");// execve() returns only on error 
	exit(EXIT_FAILURE);//exit child class
      }
      else{
	exit(EXIT_SUCCESS);//exit child class 
      }
    }
    catch(...){//catch all exception in child process
      exit(EXIT_FAILURE);//exit child class  
    }
  }
  //parent class
  return cpid;
}

void MyShellClass::PrintCommandList(){
  std::cout<<"CommandList size = "<< CommandList.size()<<std::endl;
  int i=0;
  for (std::vector<Command>::iterator it = CommandList.begin() ; it != CommandList.end(); ++it){
    std::cout << "CommandList [" <<i<<"]:"<<std::endl;
    (*it).PrintArgv();
    i++;
  }
}
int MyShellClass::SetIO(int ReadEnd, Command & com){  
  if(!(com.InputFile).empty()){//redirect intput
    int Fin= open((com.InputFile).c_str(),O_RDONLY);
    if(Fin==-1){
      perror("Error OpenInputFile:");
      throw SetIOException();
    }
    else{
      dup2(Fin,STDIN_FILENO); close(Fin);
    }
  }
  else{
    dup2(ReadEnd,STDIN_FILENO);
  }
  close(ReadEnd);//close ReadEnd of preivous command pipe
  
  if(!(com.ErrFile).empty()){ //redirect Errput
    int Ferr= open((com.ErrFile).c_str(),O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU); // 00007 others have read, write, and execute permission
    if(Ferr==-1){
      perror("openErrFile");
      throw SetIOException();
    }
    dup2(Ferr,STDERR_FILENO);
    close(Ferr);  
  }
  
  if(!(com.OutputFile).empty()){//redirect output instead of pipe
    int Fout = open((com.OutputFile).c_str(),O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU); // 00007 others have read, write, and execute permission
    if(Fout==-1){
      perror("error openOutputFile:");
      throw SetIOException();
    }
    dup2(Fout,STDOUT_FILENO);
    close(Fout);
    return -1; //indicting the next command should read from stdin no pipe
  }
  else{
    int pipefd[2];//create pipe for output
    if(pipe(pipefd)==-1){
      throw SetIOException();
    }
    dup2(pipefd[1],STDOUT_FILENO);//then output direct
    close(pipefd[1]);
    return pipefd[0];//ReadEnd of next command
  }
}

void MyShellClass::HandleSignal(int StdinNo, int StdoutNo, int StderrNo, pid_t cpid){
  int wstatus;
  pid_t w;
  //resort default IO before print signal 
  dup2(StdinNo,STDIN_FILENO); close(StdinNo);
  dup2(StdoutNo,STDOUT_FILENO);close(StdoutNo);
  dup2(StderrNo,STDERR_FILENO); close(StderrNo);
 
  //wait for the last command to end
  //get from man page of waitpid
  if(cpid>0){
    do{//from man page waitpid
      w = waitpid(cpid, &wstatus,WUNTRACED | WCONTINUED);
      //WIFSIGNALED(wstatus)
      //returns true if the child process was terminated by a signal.
      if (w == -1) {
	perror("waitpid");
	exit(EXIT_FAILURE);
      }//: on failure, waitpid returns -1;
      if (WIFEXITED(wstatus)) {
	printf("Program exited with status %d\n", WEXITSTATUS(wstatus));
      } else if (WIFSIGNALED(wstatus)) {
	printf("Program was killed by signal %d\n", WTERMSIG(wstatus));
      }
    }while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  }
}

void MyShellClass::ReadIn(){
  std::cout<<"myShell:"<<CurrentWorkingDir<<" $";  
  std::getline(std::cin,InputStr);
  if(std::cin.eof()){
    ISEXIT=true;
    if(InputStr.empty()){
      return;
    }
  }
   if(InputStr.empty()){
    throw std::invalid_argument("ReadIn:Input command is empty");
  }
  ReplaceVar();
  GetCommandList();//parse the input to get CommandList
  if(CommandList.size()==1&&CommandList[0].GetCommandName()=="exit"){
    ISEXIT=true;//parent process only exit when the input command is not pipe
  }
}

void MyShellClass::RunCommandList(){ 
  int StdinNo = dup(STDIN_FILENO);//store the default IO 
  int StdoutNo = dup(STDOUT_FILENO); 
  int StderrNo = dup(STDERR_FILENO);
  /*StdinNo, StdOutNo is the lowest value avavilable number other than 0, 1 seperately */
  int ReadEnd = dup(StdinNo);
  pid_t cpid=-1;//this initial value is uesed to show if valid cpid is set
  try{
    for(size_t i=0; i<CommandList.size();i++){      
      ReadEnd = SetIO(ReadEnd,CommandList[i]);
      if((i==CommandList.size()-1)){//last command
	if(ReadEnd!=-1){//ReadEnd = -1 shows command hs redireted output 
	  dup2(StdinNo,STDOUT_FILENO);
	}
      }
      else{
	if(ReadEnd==-1){//next command's cin is default stdin 
	  ReadEnd=dup(StdinNo);
	}
      }
      cpid= ExecuteCommand(CommandList[i]);
      dup2(StderrNo,STDERR_FILENO);//resort default std::err
    }
    if(cpid>0){//if cpid==-1,no child process is forked
    HandleSignal(StdinNo,StdoutNo, StderrNo, cpid);//wait for the last command
    }
    if(ReadEnd!=-1){
      close(ReadEnd);
    }
  }
  catch(SetIOException & err){
    if(ReadEnd!=-1){
      close(ReadEnd);
    }
    HandleSignal(StdinNo,StdoutNo, StderrNo, cpid);
    std::cout<<err.what()<<std::endl;
    return;
  }
}
