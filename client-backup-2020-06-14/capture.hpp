/*
MIT License

Copyright (c) 2020 Robert T. Adams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef capture_hpp_20200613_185115_PDT
#define capture_hpp_20200613_185115_PDT

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>

namespace /* anon */  {
 

} /* namespace anon */

class Capture
{
public:
  Capture(const std::string& in_path, const std::string& out_path) -> void
    {
      using namespace std;
      cout << "Setting TTY options and port speed...";
      init_opts(in_path);
      cout << "Done." << endl;
      cout << "Opening port \"" << in_path << "\"" << endl;
      if(port == -1)
      {
        throw std::runtime_error("Could not open port");
      }
      int read_timeout = 3;     // number of seconds before read operations on the port time out
                                // this value should be large enough to allow Arduino to boot
                                // before reading from serial port
      auto port = open(in_path.c_str(), O_RDWR);
      string inbuf;
      auto readbuf = [&] 
        {
          constexpr int bufsize = 255;
          array<char, bufsize> buf;
          auto poll_result = poll(port, 1, read_timeout);
          if(poll_result > 0)
          {
            auto result = read(port, buf.data(), bufsize);
            inbuf.append(buf.data(), bufsize);
            return result;
          }
          else if(poll_result < 0)
          {
            throw runtime_error("ERROR: poll() returned error")
          }
          else
          {
            throw runtime_error("ERROR: port read operation timeout");
          }
        };
      readbuf();
      if(inbuf != "@ready\n")
      {
        throw std::runtime_error(
      }
      cout << "Sending 'scan' command to port..." << endl;
      string cmd;
      cmd = "scan\n";
      write(port, cmd.c_str(), cmd.size());
      string msg;
      bool end_of_stream = false;
      while(end_of_stream == false)
      {
        
      }
      close(port);
      cout << "Port closed." << endl;
    }
  Capture()
    {
    }

private:
  termios saved_termios_;

  auto init_opts() -> void
    {
      using namespace std;
      // set termios to match these settings:
      // stty -F /dev/ttyUSB0 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost
      // -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh
      // -ixon -crtscts 
      termios new_options;
      
      //if(tcsetattr(file, TCSANOW, &options) == -1)
      //{
      //  throw std::runtime_error("Could not set port options; tcsetattr failed");
      //}
      auto baud = B115200;
      if(cfsetispeed(term, baud) != baud || cfsetospeed(term, baud) != baud)
      {
        throw std::runtime_error("Could not set baud rate");
      }
    }
  auto init_baud(termios* term, speed_t baud)
    {
    }
};

#endif//capture_hpp_20200613_185115_PDT
