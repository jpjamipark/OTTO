#include "engine_manager.hpp"

#include "core/engine/engine_dispatcher.hpp"
#include "core/engine/engine_dispatcher.inl"
#include "core/ui/vector_graphics.hpp"

#include "engines/fx/wormhole/wormhole.hpp"
#include "engines/fx/chorus/chorus.hpp"
#include "engines/synths/OTTOFM/ottofm.hpp"
#include "engines/synths/goss/goss.hpp"


#include "services/application.hpp"
#include "services/clock_manager.hpp"

namespace otto::services {

  using namespace core;
  using namespace core::engine;

  struct DefaultEngineManager final : EngineManager {
    DefaultEngineManager();

    void start() override;
    audio::ProcessData<2> process(audio::ProcessData<1> external_in) override;

  private:
    // using EffectsDispatcher = EngineDispatcher< //
    //   EngineType::effect>;
    // using ArpDispatcher = EngineDispatcher< //
    //   EngineType::arpeggiator>;
    using SynthDispatcher = EngineDispatcher< //
      EngineType::synth,
      engines::ottofm::OttofmEngine,
      engines::goss::GossEngine>;

    SynthDispatcher synth;
    engines::wormhole::Wormhole effect1;
    engines::chorus::Chorus effect2;
    // ArpDispatcher arpeggiator{true};
    // EffectsDispatcher effect1{true};
    // EffectsDispatcher effect2{true};

    // engines::Sends synth_send;
    // engines::Sends line_in_send;
    // engines::Master master;
    // engines::Sequencer sequencer;
  };

  std::unique_ptr<EngineManager> EngineManager::create_default()
  {
    return std::make_unique<DefaultEngineManager>();
  }

  DefaultEngineManager::DefaultEngineManager()
  {
    auto& ui_manager = *Application::current().ui_manager;
    auto& state_manager = *Application::current().state_manager;
    auto& controller = *Application::current().controller;

    auto reg_ss = [&](auto se, auto&& f) { return ui_manager.register_screen_selector(se, f); };

    // reg_ss(ScreenEnum::sends, [&]() -> auto& { return synth_send.screen(); });
    // reg_ss(ScreenEnum::routing, );
    reg_ss(ScreenEnum::fx1, [&]() { return effect1.screen(); });
    // reg_ss(ScreenEnum::fx1_selector, [&]() -> auto& { return effect1.selector_screen(); });
    reg_ss(ScreenEnum::fx2, [&]() { return effect2.screen(); });
    // reg_ss(ScreenEnum::fx2_selector, [&]() -> auto& { return effect2.selector_screen(); });
    // reg_ss(ScreenEnum::looper,         [&] () -> auto& { return  ; });
    // reg_ss(ScreenEnum::arp, [&]() -> auto& { return arpeggiator->screen(); });
    // reg_ss(ScreenEnum::arp_selector, [&]() -> auto& { return arpeggiator.selector_screen(); });
    // reg_ss(ScreenEnum::master, [&]() -> auto& { return master.screen(); });
    // reg_ss(ScreenEnum::sequencer, [&]() -> auto& { return sequencer.screen(); });
    // reg_ss(ScreenEnum::sampler, [&]() -> auto& { return sequencer.sampler_screen(); });
    // reg_ss(ScreenEnum::sampler_envelope, [&]() -> auto& { return sequencer.envelope_screen(); });
    reg_ss(ScreenEnum::synth, [&]() { return synth->screen(); });
    reg_ss(ScreenEnum::synth_selector, [&]() { return synth.selector_screen(); });
    reg_ss(ScreenEnum::synth_envelope, [&]() { return synth->envelope_screen(); });
    reg_ss(ScreenEnum::voices, [&]() { return synth->voices_screen(); });
    // reg_ss(ScreenEnum::external,       [&] () -> auto& { return  ; });
    // reg_ss(ScreenEnum::twist1,         [&] () -> auto& { return  ; });
    // reg_ss(ScreenEnum::twist2,         [&] () -> auto& { return  ; });

    ui_manager.state.current_screen.on_change().connect([&](auto new_val) {
      switch (new_val) {
        case ScreenEnum::synth: [[fallthrough]];
        case ScreenEnum::synth_envelope: [[fallthrough]];
        case ScreenEnum::synth_selector: [[fallthrough]];
        case ScreenEnum::voices:
          if (ui_manager.state.active_channel != +ChannelEnum::internal)
            ui_manager.state.active_channel = +ChannelEnum::internal;
        default: break;
      }
    });

    // controller.register_key_handler(input::Key::sequencer, [&](input::Key k) {
    // ui_manager.display(ScreenEnum::sequencer);
    // });

    // controller.register_key_handler(input::Key::sampler, [&](input::Key k) { ui_manager.display(ScreenEnum::sampler);
    // });

    // controller.register_key_handler(input::Key::arp, [&](input::Key k) {
    //   if (controller.is_pressed(input::Key::shift)) {
    //     ui_manager.display(ScreenEnum::arp_selector);
    //   } else {
    //     ui_manager.display(ScreenEnum::arp);
    //   }
    // });

    controller.register_key_handler(input::Key::synth, [&](input::Key k) {
      if (controller.is_pressed(input::Key::shift)) {
        ui_manager.display(ScreenEnum::synth_selector);
      } else {
        ui_manager.display(ScreenEnum::synth);
      }
    });

    controller.register_key_handler(input::Key::envelope, [&](input::Key k) {
      if (ui_manager.state.active_channel == ChannelEnum::internal) {
        if (controller.is_pressed(input::Key::shift)) {
          ui_manager.display(ScreenEnum::voices);
        } else {
          ui_manager.display(ScreenEnum::synth_envelope);
        }
      } else {
        ui_manager.display(ScreenEnum::sampler_envelope);
      }
    });

    controller.register_key_handler(input::Key::fx1, [&](input::Key k) {
      if (controller.is_pressed(input::Key::shift)) {
        // ui_manager.display(ScreenEnum::fx1_selector);
      } else {
        ui_manager.display(ScreenEnum::fx1);
      }
    });

    controller.register_key_handler(input::Key::fx2, [&](input::Key k) {
      if (controller.is_pressed(input::Key::shift)) {
        // ui_manager.display(ScreenEnum::fx2_selector);
      } else {
        ui_manager.display(ScreenEnum::fx2);
      }
    });

    // static ScreenEnum master_last_screen = ScreenEnum::master;
    // static ScreenEnum send_last_screen = ScreenEnum::sends;

    // controller.register_key_handler(
    //   input::Key::master,
    //   [&](input::Key k) {
    //     master_last_screen = ui_manager.state.current_screen;
    //     ui_manager.display(ScreenEnum::master);
    //   },
    //   [&](input::Key k) {
    //     if (master_last_screen) ui_manager.display(master_last_screen);
    //   });

    // controller.register_key_handler(
    //   input::Key::sends,
    //   [&](input::Key k) {
    //     send_last_screen = ui_manager.state.current_screen;
    //     ui_manager.display(ScreenEnum::sends);
    //   },
    //   [&](input::Key k) {
    //     if (send_last_screen) ui_manager.display(send_last_screen);
    //   });

    auto load = [&](nlohmann::json& data) {
      util::deserialize(synth, data["Synth"]);
      // effect1.from_json(data["Effect1"]);
      // effect2.from_json(data["Effect2"]);
      // master.from_json(data["Master"]);
      // arpeggiator.from_json(data["Arpeggiator"]);
      // sequencer.from_json(data["Sequencer"]);
    };

    auto save = [&] {
      return nlohmann::json({
        {"Synth", util::serialize(synth)},
        // {"Effect1", effect1.to_json()},
        // {"Effect2", effect2.to_json()},
        // {"Master", master.to_json()},
        // {"Arpeggiator", arpeggiator.to_json()},
        // {"Sequencer", sequencer.to_json()},
      });
    };

    state_manager.attach("Engines", load, save);
  }

  void DefaultEngineManager::start() {}

  audio::ProcessData<2> DefaultEngineManager::process(audio::ProcessData<1> external_in)
  {
    // Main processor function
    auto midi_in = external_in.midi_only();
    midi_in.clock = ClockManager::current().step_frames(external_in.nframes);
    // auto arp_out = arpeggiator->process(midi_in);
    auto synth_out = synth.process(external_in);
    auto right_chan = Application::current().audio_manager->buffer_pool().allocate();

    util::copy(synth_out.audio, right_chan.begin());

    auto fx1_bus = Application::current().audio_manager->buffer_pool().allocate();
    auto fx2_bus = Application::current().audio_manager->buffer_pool().allocate();

    for (auto&& [snth, fx1, fx2] : util::zip(synth_out.audio, fx1_bus, fx2_bus)) {
      fx1 = snth * 0.25; // * synth_send.props.to_FX1;
      fx2 = snth * 0.25; // * synth_send.props.to_FX2;
    }
    auto fx1_out = effect1.audio->process(audio::ProcessData<1>(fx1_bus));
    auto fx2_out = effect2.audio->process(audio::ProcessData<1>(fx2_bus));

    // Temporary. Get only synth output for testing
    for (auto&& [snth, l, r] : util::zip(synth_out.audio, fx1_out.audio[0], fx1_out.audio[1])) {
      l = r = snth;
    }
    return fx1_out;
    // // Sequencer. Outputs L/R dry output and adds to fx busses.
    // // auto seq_dry = sequencer.process(midi_in, fx1_bus, fx2_bus);
    // auto fx1_out = effect1->process(audio::ProcessData<1>(fx1_bus));
    // auto fx2_out = effect2->process(audio::ProcessData<1>(fx2_bus));

    // Stereo output gathered in fx1_out
    for (auto&& [snth, fx1L, fx1R, fx2L, fx2R] :
         util::zip(synth_out.audio, fx1_out.audio[0], fx1_out.audio[1], fx2_out.audio[0], fx2_out.audio[1])) {
      fx1L += fx2L + snth * 0.5; // * synth_send.props.dry * (1 - synth_send.props.dry_pan);
      fx1R += fx2R + snth * 0.5; // * synth_send.props.dry * (1 + synth_send.props.dry_pan);
    }

    return fx1_out;
    // synth_out.audio.release();
    // fx2_out.audio[0].release();
    // fx2_out.audio[1].release();
    // fx1_bus.release();
    // fx2_bus.release();


    // auto smplr_out = sequencer.process(midi_in);
    // for (auto&& [resL, resR, smplr] : util::zip(fx1_out.audio[0], fx1_out.audio[1], smplr_out.audio)) {
    //   resL += smplr;
    //   resR += smplr;
    // }


    // return master.process(std::move(fx1_out));
    /*
    auto temp = Application::current().audio_manager->buffer_pool().allocate_multi_clear<2>();
    for (auto&& [in, tmp] : util::zip(seq_out, temp)) {
    std::get<0>(tmp) += std::get<0>(in);
        std::get<1>(tmp) += std::get<0>(in);
        }
    return master.process({std::move(temp),external_in.midi,external_in.nframes});
    */
  }

} // namespace otto::services
