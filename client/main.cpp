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
    return 1;
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
    auto capture = Capture(port, output); 
  }
  catch(const std::exception& e)
  {
    using namespace std;
    cout << e.what() << "\n"
         << "\n"
         << "Use \"--help\" to display program options.\n"
         << endl;
  }
}
