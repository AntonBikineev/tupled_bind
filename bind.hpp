#include <tuple>

// That is my tuple-based experimental bind implementation.

namespace my
{

  namespace placeholders
  {
    template <unsigned n>
    using index = std::integral_constant<unsigned, n>;

    namespace
    {
      index<1u> _1;
      index<2u> _2;
      index<3u> _3;
      index<4u> _4;
      index<5u> _5;
    }
  }

  namespace detail
  {
    template <unsigned... Ns>
    struct sequence{};

    template <unsigned N, unsigned... Rest>
    struct ascending_sequence: ascending_sequence<N - 1, N - 1, Rest...>{};

    template <unsigned... Rest>
    struct ascending_sequence<0u, Rest...>
    {
      using type = sequence<Rest...>;
    };

    template <class... T, class TupleArgs>
    constexpr auto merge_tuples_imp(const std::tuple<T...>& d2, const TupleArgs& tuple_args)
    {
      return d2;
    }

    template <class... T1, class T2, class... T3, class TupleArgs>
    constexpr auto merge_tuples_imp(const std::tuple<T1...>& d2, const TupleArgs& tuple_args, const T2& head, const T3&... rest)
    {
      const auto concated = std::tuple_cat(d2, std::make_tuple(head));

      return merge_tuples_imp(concated, tuple_args, rest...);
    }

    template <class... T1, unsigned N, class... T2, class TupleArgs>
    constexpr auto merge_tuples_imp(const std::tuple<T1...>& d2, const TupleArgs& tuple_args, placeholders::index<N> head, const T2&... rest)
    {
      const unsigned index = N - 1;
      const auto concated = std::tuple_cat(d2, std::make_tuple(std::get<index>(tuple_args)));

      return merge_tuples_imp(concated, tuple_args, rest...);
    }

    template <class... T1, class TupleArgs, unsigned... Ns>
    constexpr auto merge_tuples_imp(const std::tuple<T1...>& d1, const TupleArgs& tuple_args, sequence<Ns...>)
    {
      return merge_tuples_imp(std::tuple<>{}, tuple_args, std::get<Ns>(d1)...);
    }

    template <class... T1, class TupleArgs>
    constexpr auto merge_tuples(const std::tuple<T1...>& d1, const TupleArgs& tuple_args)
    {
      return merge_tuples_imp(d1, tuple_args, typename ascending_sequence<sizeof...(T1)>::type{});
    }

    template <class F, class... Args, unsigned... Ns>
    auto call_function_with_tupled_arguments_imp(F f, const std::tuple<Args...>& tuple, sequence<Ns...>)
    {
      return f(std::get<Ns>(tuple)...);
    }

    template <class F, class... Args>
    auto call_function_with_tupled_arguments(F f, const std::tuple<Args...>& tuple)
    {
      return call_function_with_tupled_arguments_imp(f, tuple, typename ascending_sequence<sizeof...(Args)>::type{});
    }

    template <class F, class... Args>
    class bind_t
    {
      std::tuple<Args...> args;
      F f;
    public:
      bind_t(F f, Args... args):
        f(f), args(std::make_tuple(args...))
      {
      }

      template <class... NewArgs>
      auto operator()(NewArgs... newargs)
      {
        const std::tuple<NewArgs...> tuple_args = std::make_tuple(newargs...);

        const auto merged_tuple = merge_tuples(args, tuple_args);
        return call_function_with_tupled_arguments(f, merged_tuple);
      }
    };
  }

template <class F, class... Args>
auto bind(F f, Args... args)
{
  return detail::bind_t<F, Args...>(f, args...);
}

}
