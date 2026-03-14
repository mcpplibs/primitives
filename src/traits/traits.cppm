module;

export module mcpplibs.primitives.traits;

export import mcpplibs.primitives.traits.underlying;
export import mcpplibs.primitives.traits.policy;

// Expose default policy aliases from the primitives-facing traits module.
export namespace mcpplibs::primitives {
using default_value_policy = policy::unchecked_value;
using default_type_policy = policy::transparent_type;
using default_error_policy = policy::throw_error;
using default_concurrency_policy = policy::single_thread;
} // namespace mcpplibs::primitives
