#include <string>
#include <fstream>  
#include <iostream>
/*From C++ referencs:
  http://www.cplusplus.com/reference/fstream/ofstream/open/
*/
int main(){
  std::ofstream ofs;
  std::string o = "This is larger data";
  ofs.open ("LargeData.txt", std::ofstream::out);
  for(size_t i=0; i< 75536;i++){
    ofs << o<<std::endl;
    std::cerr<<o<<std::endl;
  }
  ofs.close();
  return 0;
}
