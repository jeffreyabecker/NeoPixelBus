
- [ ] Consolidate `IPixelBus` types now that colors have been refactored.

- [ ] Remove transport tags and one-wire-specific transport requirements.
	- [ ] Audit current usage of `AnyTransportTag`, `TransportTag`, `OneWireTransportTag`, and transport capability checks across protocols/transports/factory traits.
	- [ ] Replace tag-based protocol/transport compatibility constraints with a single transport-shape contract at seam boundaries.
	- [ ] Delete one-wire-specific trait branches and descriptor requirements now that one-wire encoding/wrapping is transport-external.
	- [ ] Refactor factory/descriptor plumbing to remove tag-gated overload paths while preserving explicit settings normalization.
	- [ ] Update protocol aliases/descriptors/examples so they no longer encode transport-tag coupling.
	- [ ] Update and tighten compile-contract suites to validate the new non-tag compatibility model.
	- [ ] Validate `pio test -e native-test` and `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile` after cleanup.

- [ ] Centralize buffer access for bus/protocol/transport/shader layers.
	- [ ] Introduce a per-bus buffer-access object that is passed by reference instead of passing multiple `BufferHolder` instances.
	- [ ] Expose typed accessors for root pixel buffer, shader scratch buffer, protocol arena, and per-strand protocol slices.
	- [ ] Add a common buffer-access provider interface so composite busses expose the same buffer surface as single busses.
	- [ ] Consolidate protocol-buffer sizing/binding logic into one reusable binder shared by static/dynamic owning bus implementations.
	- [ ] Migrate in phases: add adapters first, switch internals/factories next, then remove legacy multi-`BufferHolder` constructor paths.


- [ ] Add bus-level config for refresh coordination (`fullRefreshOnly` / wait for all transports to finish).
- [ ] Support non-reallocating settings alteration and expose common interfaces through composite busses.