
Creating a module that can be used with the audio system:

1. Derive from sndbx::audio::Patchable and pass the number of inputs and outputs to its constructor. 
   This example is for a sine wave with an FM input and adjustable volume. 

```cpp
  using namespace sndbx;

  class MyModule : public audio::Patchable {
    AudioSynthWaveformModulated synth_;
    AudioGraph::NodeID synth_id_;
    AudioAmplifier amp_;
    AudioGraph::NodeID amp_id_;
  public:
    MyModule() : Patchable{1, 1} {} //1 input, 1 output
    
  // class cont. below
```

2. Override link() to connect to the audio graph. In this example, the module internally holds two audio nodes (synth_ and amp_ above) that are connected together. Adding or connecting nodes can fail (graph is full, etc.) so 
we forward any errors to the caller of link() and clean up partial successes. If successful, the ids are stored (needed for unlink());

```cpp
  using namespace sndbx;

  AudioError link(AudioGraph &graph) override {
    auto s_id = graph.add_node(&synth_);  
    if (!s_id) {
      return s_id.error();
    }
    synth_id_ = *s_id;

    auto a_id = graph.add_node(&amp_);
    if (!a_id) {
      // clean up previous
      graph.remove_node(synth_id_);
      return a_id.error();
    }
    amp_id_ = *a_id;
    
    // final step, so we can just return any error from it directly
    return graph.connect(synth_id_, 0, amp_id_, 0);
  }

  // class continued below...
```

2. Provide the reverse of link(), i.e. every add_node() must have a corresponding remove_node().
   All related connections will be deleted, so disconnect() does not need to be called. Note that 
   synth_id_ and amp_id_ are invalidated after this operation (see below). 
```cpp
  using namespace sndbx;

  void unlink(AudioGraph &graph) override { 
    graph.remove_node(amp_id_);
    graph.remove_node(synth_id_);
  }
  
  /* Note: IDs are now invalid ... but the actual AudioStreams stored
   * in our class are unaffected, so the ids could be reseated (but in the system
   * link() and unlink() are only called once)
   */
```

3. Provide the patching points. This function should assume that port.index is valid.
   The structure of the example module is the following:

    [IN]   <- Module input
      |
   [synth]
      |    <- internal audio graph connection
    [amp]
      |
    [OUT]  <- Module output

    The two patch points are the input of [synth] and the output of [amp]. map() defines the mapping
    between the logical module port and the actual point in the audio graph. The first field in the endpoint is the AudioStream's audio graph id, and the second field is the AudioStream port number that should be used. 
    port.type indicates whether the query is for an input or an output. 
   
```cpp
  using namespace sndbx;

  AudioEndpoint map(ModulePort port) const override {
    // Mapping: Module input  0 -> synth input 0
    //          Module output 0 -> amp output  0
    assert(port.index == 0);
    switch (port.type) {
      case ModulePort::Type::IN:
        return audio::make_endpoint(synth_id_, 0);
      case ModulePort::Type::OUT: 
        return audio::make_endpoint(amp_id_, 0); 
    }
  }
```