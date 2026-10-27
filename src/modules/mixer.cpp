
// #include "mixer.hpp"
// #include "audio/audio_trigger.hpp"
// #include "controllable.hpp"
// #include "displayable.hpp"
// #include "module_display_info.hpp"
// #include "module_parameter.hpp"


// namespace sndbx {
// class Mixer;

// template <>
// inline constexpr ModuleDisplayInfo module_info<Mixer>{
//     {"mixer", 0x88FF09},
//     "4 channel mixer",
//     {"gain 1", "gain 2", "gain 3", "gain 4"},
//     {"1", "2", "3", "4"},
//     {"out"},
//     {ctrl_format<FMillis>, ctrl_format<FMillis>, ctrl_format<FMillis>,
//      ctrl_format<FMillis>}};

// class Mixer : public Patchable, public Controllable, public Displayable {
// 	using Self = Mixer;

//   public:
// 	Mixer()
// 	    : Patchable{num_ins<Self>, num_outs<Self>},
// 	      Controllable{num_ctrls<Self>} {}

// 	AudioError link(AudioGraph &graph) override {
// 		return audio::add_nodes(graph, {&mix_, mix_id_});
// 	}

// 	void unlink(AudioGraph &graph) override { graph.remove(mix_id_); }

// 	AudioEndpoint patch_point(ModulePort port) const override {
// 		return audio::make_endpoint(mix_id_, port.index);
// 	}

// 	float change_control(ControlChange c) override {
// 		assert(c.index < num_controls());
// 		return gain_ctrl.change(*this, channel_gains_[c.index], c.delta);
// 	}

// 	const ModuleDisplayInfo &display_info() const override {
// 		return module_info<Self>;
// 	}

//   private:
// 	void update_gains() {
// 		for (std::size_t ch{}; ch < channel_gains_.size(); ++ch) {
// 			mix_.gain(ch, channel_gains_[ch]);
// 		}
// 	}

// 	constexpr static auto channels = num_ins<Self>;
// 	using ChannelGains = std::array<ModuleParameter<float>, channels>;

// 	constexpr static auto default_gain = ModuleParameter{0.0f, 1.0f, 0.5f};

// 	template <std::size_t... Is>
// 	static ChannelGains make_gain_params_impl(std::index_sequence<Is...>) {
// 		ChannelGains{(((void)Is, default_gain))...};
// 	}

// 	static ChannelGains make_gain_params() {
// 		return make_gain_params_impl(std::make_index_sequence<channels>{});
// 	}

// 	struct gain_coeff {
// 		constexpr static auto value{1.1f};
// 	};

// 	AudioMixer4 mix_;
// 	nst::teensy::AudioNodeID mix_id_;

// 	ChannelGains channel_gains_{make_channel_gains()};

// 	constexpr static auto gain_ctrl =
// 	    ModuleControl{curves::linear<float, gain_coeff>, &Self::update_gains};
// };

// } // namespace sndbx

// #endif