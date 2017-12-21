#include "my_shell_class.h"
#include <cstdio>
#include <cstdlib>
#include <exception>

std::map <std::string,std::string> MyShellClass::LocalVars;
extern char **environ;

int main(){
  do{
    MyShellClass myshell(environ);
    try{
    myshell.ReadIn();
    if(myshell.IsExit()){//encounter EOF or exit
      return EXIT_SUCCESS;
    }
    //myshell.PrintCommandList();//print all command and their argumnets 
    if(myshell.CommandNum()!=0){
      myshell.RunCommandList();
    }
    
    if(myshell.IsExit()){
       return EXIT_SUCCESS;
     }
    }
    catch(std::invalid_argument & err){//all exception happens in readin
      std::cout<<err.what()<<std::endl;
      continue;
    }
    catch(std::exception & err){
      std::cout<<err.what()<<std::endl;
      continue;
    }
  }while(1);
  return EXIT_SUCCESS;
}
