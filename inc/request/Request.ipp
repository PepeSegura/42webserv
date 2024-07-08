// Copyright 2024 42Madrid GPL
// Author: alvjimen
#include "Request.hpp"
template<typename T>
std::string request::Handler::to_string(T value)
{
  std::ostringstream oss;
  oss << value;
  return oss.str();
}
/*
template <typename Iterator1, typename Iterator2>
Iterator1 find(Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2)
{
   return std::search(first1, last1, first2, last2);
}
*/
