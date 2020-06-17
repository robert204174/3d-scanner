#ifndef capture_hpp_20200613_185115_PDT
#define capture_hpp_20200613_185115_PDT

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

class Capture
{
public:
  Capture(const std::string& pp, const std::string& output_file_path)
      : port_path_(pp)
    {
      using namespace std;
      init_opts(pp);
      fstream port_file(pp, port_file.in | port_file.out);
      port_file << "scan" << endl;
      fstream outfile(output_file_path, outfile.out | outfile.trunc);
      string line;
      while(port_file)
      {
        getline(port_file, line);
        outfile << line; 
      }
    }
  ~Capture()
    {
      restore_opts();
    }

private:
  termios       saved_options_;
  std::string   port_path_;
  auto init_opts(const std::string& path) -> void
    {
      using namespace std;
      auto fd = open(path.c_str(), O_RDWR);
      if(fd == -1)
      {
        string msg = "Could not open port for input: ";
        msg += path;
        throw runtime_error(msg);
      }
      // set termios to match these settings:
      // stty -F /dev/ttyUSB0 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost
      // -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh
      // -ixon -crtscts 
      termios options;
      if(tcgetattr(fd, &saved_options_) != 0)
      {
        throw std::runtime_error("Could not save TTY options: tcgetattr() failed");
      }
      tcgetattr(fd, &options);
      options.c_cflag &= (~HUPCL);    // prevent arduino reboot on connection
                                      // serial port
      const speed_t baud = B115200;
      if(cfsetspeed(&options, baud) != 0)
      {
        throw std::runtime_error("Could not set baud rate");
      }
      if(tcsetattr(fd, TCSANOW, &options) == -1)
      {
        throw std::runtime_error("Could not set port options; tcsetattr failed");
      }
      close(fd);
    }
  auto restore_opts() -> void
    { 
      using namespace std;
      auto fd = open(port_path_.c_str(), O_RDWR);
      if(fd != -1)
      {
        if(tcsetattr(fd, TCSANOW, &saved_options_) == -1)
        {
          cout << "WARNING: Could not restore saved port options; tcsetattr failed" << endl;
        }
      } 
      if(fd != -1)
      {
        close(fd);
      }
    }
};

#endif//capture_hpp_20200613_185115_PDT
