#ifndef TICTOC_HPP_INC
#define TICTOC_HPP_INC
#include <chrono>
#include<iostream>

class notictoc{
public:
notictoc(std::string ignore){};};

class tictoc
{
  std::chrono::time_point<std::chrono::steady_clock> start;
  std::string _scope;
  public:
   tictoc(std::string scope): _scope(scope)
   {
      start = std::chrono::steady_clock::now();
   }
   ~tictoc()
   {
      using namespace std::literals;
	 const auto end = std::chrono::steady_clock::now();
 
    std::cout
      << "Scope " << _scope << " took "
      << (end - start) / 1ms << "ms ≈ " // almost equivalent form of the above, but
      << (end - start) / 1s << "s.\n";  // using milliseconds and seconds accordingly
   }
};
#endif
