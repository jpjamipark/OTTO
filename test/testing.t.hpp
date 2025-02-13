#pragma once
#include <fmt/format.h>

#include <chrono>
#include <fstream>
#include <random.hpp>
#include <sstream>
#include <unordered_map>

#include "services/log_manager.hpp"
#include "util/algorithm.hpp"
#include "util/filesystem.hpp"
#include "util/type_traits.hpp"
#include "util/utility.hpp"

#define CATCH_CONFIG_ENABLE_BENCHMARKING 1
#include <catch.hpp>

#include "benchmark_csv_reporter.hpp"
#include "graphics.t.hpp"

using Random = effolkronium::random_static;

namespace otto::test {
  inline fs::path dir = fs::current_path() / "testdir";

  inline void truncateFile(const filesystem::path& p)
  {
    std::fstream fstream;
    fstream.open(p.c_str(), std::ios::trunc | std::ios::out | std::ios::binary);
    fstream.close();
  }

  struct measure {
    using TimeT = std::chrono::nanoseconds;

    template<typename F, typename... Args>
    static auto execution(F&& func, Args&&... args)
    {
      auto start = std::chrono::steady_clock::now();
      std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
      return std::chrono::duration_cast<TimeT>(std::chrono::steady_clock::now() - start);
    }
  };

  inline auto float_cmp(float margin)
  {
    return [margin](float a, float b) { return std::abs(a - b) < margin; };
  }

  struct approx {
    approx(float v) noexcept : value_(v) {}

    approx& margin(float m) noexcept
    {
      margin_ = m;
      return *this;
    };

    bool operator==(float rhs) const noexcept
    {
      return std::abs(value_ - rhs) < margin_;
    }

    friend bool operator==(float lhs, const approx& rhs)
    {
      return (rhs == lhs);
    }

    bool operator!=(float rhs) const noexcept
    {
      return !(*this == rhs);
    }

    friend bool operator!=(float lhs, const approx& rhs)
    {
      return rhs != lhs;
    }

    friend std::ostream& operator<<(std::ostream& os, const approx& a)
    {
      os << "approx(" << a.value_ << ", ±" << a.margin_ << ")";
      return os;
    }

  private:
    float value_;
    float margin_ = 0.0001;
  };

  template<typename Cont, typename Proj>
  auto sort(Cont&& c, Proj&& projection)
  {
    auto vec = util::view::to_vec(c);
    util::sort(vec, [&](auto&& a, auto&& b) { return projection(a) < projection(b); });
    return vec;
  }

  template<typename Cont>
  auto sort(Cont&& c)
  {
    return test::sort(c, util::identity);
  }

} // namespace otto::test

namespace Catch {
  template<typename... Args>
  struct StringMaker<std::tuple<Args...>> {
    static std::string convert(std::tuple<Args...> const& value)
    {
      if constexpr (sizeof...(Args) == 0) return "{}";
      std::ostringstream o;
      o << "{";
      otto::util::for_each(
        value, [&](const auto& a) { o << StringMaker<std::decay_t<decltype(a)>>::convert(a) << ", "; });
      auto str = o.str();
      // Chop the extra ", "
      str.resize(str.size() - 2);
      return str + "}";
    }
  };
} // namespace Catch
