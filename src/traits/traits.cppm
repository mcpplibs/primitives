module;

export module mcpplibs.primitive.traits;

export import mcpplibs.primitive.traits.underlying;
export import mcpplibs.primitive.traits.policy;

// Expose default policy aliases from the primitives-facing traits module.
export namespace mcpplibs::primitives {
using default_value_policy = ::mcpplibs::primitive::policy::unchecked_value;
using default_type_policy = ::mcpplibs::primitive::policy::transparent_type;
using default_error_policy = ::mcpplibs::primitive::policy::throw_error;
using default_concurrency_policy = ::mcpplibs::primitive::policy::single_thread;
} // namespace mcpplibs::primitives
