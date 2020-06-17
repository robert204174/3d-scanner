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
#include "capture.hpp"
#include <string>
#include <iostream>
#include <boost/program_options.hpp>

int main(int argc, char* argv[])
{
  using namespace std;
  namespace po = boost::program_options;
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("port,p", po::value<string>(), "serial port path")
      ("output,o", po::value<string>(), "output file")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  auto show_help = [&]
    {
      cout << desc << "\n";
      return 1;
    };

  if (vm.count("help"))
  {
    show_help();
  }
  bool using_standard_output = false;
  string port;
  string output;
  try
  {
    // input port
    if(vm.count("port") == 0)
    {
      throw runtime_error("Port not specified");
    }
    else
    {
      port = vm["port"].as<string>();
    }
    // output file
    if(vm.count("output") == 0)
    {
      using_standard_output = true;
    }
    else
    {
      output = vm["output"].as<string>();
    }
  }
  catch(const std::exception& e)
  {
    using namespace std;
    cout << e.what() << "\n"
         << "\n"
         << "Use \"--help\" to display program options.\n"
         << endl;
  }
  capture(port, output); 
}
