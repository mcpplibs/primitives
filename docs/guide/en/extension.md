# Extension Guide

This page covers extensibility paths used by advanced users.

## 1) Custom Underlying Type

Define a domain type and specialize `underlying::traits<T>`.

Required members:

- `value_type`
- `rep_type`
- `static constexpr bool enabled`
- `static constexpr underlying::category kind`
- `to_rep(value)`
- `from_rep(rep)`
- `is_valid_rep(rep)`

After registration, your type can be used as `primitive<YourType, ...>` if it satisfies `underlying_type`.

Reference example: `examples/ex08_custom_underlying.cpp`.

## 2) Custom Common Rep Negotiation

If default `std::common_type_t` is not suitable, specialize:

```cpp
template <>
struct mcpplibs::primitives::underlying::common_rep_traits<LhsRep, RhsRep> {
  using type = YourCommonRep;
  static constexpr bool enabled = true;
};
```

This affects mixed dispatch and type negotiation.

## 3) Custom Policy Tags and Handlers

Custom policies require:

1. Registering tags via `policy::traits<YourPolicyTag>`.
2. Providing protocol specializations in the correct namespaces:
   - `policy::type::handler`
   - `policy::value::handler`
   - `policy::error::handler`
   - `policy::concurrency::handler`

If your value policy needs operation runtime behavior, also provide `operations::runtime::op_binding` specializations.

Reference example: `examples/ex09_custom_policy.cpp`.

## 4) Custom Operation Tags

To add new operation tags:

1. Define operation tag type.
2. Specialize `operations::traits<OpTag>` with:
   - `enabled = true`
   - `arity`
   - `capability_mask`
3. Provide `operations::runtime::op_binding<OpTag, ValuePolicy, CommonRep>`.
4. Invoke via `operations::apply<OpTag>(lhs, rhs)`.

Reference example: `examples/ex10_custom_operation.cpp`.

## Extension Checklist

- Policy groups are consistent across operands.
- Operation capability metadata is valid.
- Value policy + operation binding exists for runtime dispatch.
- Error policy handler returns expected payload type.
- Concurrency access handler exists if you need `load/store/CAS`.

## Next

- API details for protocol contracts: [../../api/en/README.md](../../api/en/README.md)

