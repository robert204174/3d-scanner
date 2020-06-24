#ifndef fake_pair_hpp_20200617_204432_PDT
#define fake_pair_hpp_20200617_204432_PDT

template<typename T1, typename T2>
struct pair
{
  using first_type    = T1;
  using second_type   = T2;

  first_type    first;
  second_type   second;
    
  template< class U1, class U2 >
  constexpr pair( U1&& x, U2&& y )
  : first(x)
  , second(y)
    {}

};

#endif//fake_pair_hpp_20200617_204432_PDT
