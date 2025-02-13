#include "dummy_services.hpp"
#include "engines/synths/OTTOFM/audio.hpp"
#include "testing.t.hpp"

namespace otto::engines::ottofm {

  using namespace services;

  TEST_CASE ("FM Benchmarks", "[.benchmarks]") {
    std::array<itc::Shared<float>::Storage, 4> activities;
    Audio audio{{
      activities[0],
      activities[1],
      activities[2],
      activities[3],
    }};
    Voice& v = audio.voice_mgr_.voices()[0];
    auto app = services::test::make_dummy_application();
    audio.voice_mgr_.handle_midi(midi::NoteOnEvent(60));
    auto buf = AudioManager::current().buffer_pool().allocate_clear();

    BENCHMARK ("Voice operator() inner switch lambda") {
      v.process({buf});
    };
  }

} // namespace otto::engines::ottofm
