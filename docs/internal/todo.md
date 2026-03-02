
- [ ] Consolidate `IPixelBus` types now that colors have been refactored.
- [ ] Rip out `ResourceHandle`.
	- [ ] Audit all remaining `ResourceHandle` call sites and classify ownership intent (owning, borrowing, optional).
	- [ ] Replace `ResourceHandle` in public/virtual-first seams with explicit C++17 ownership forms already used by the bus/protocol/transport architecture.
	- [ ] Refactor factories/builders so construction paths no longer depend on `ResourceHandle` wrappers.
	- [ ] Remove dead compatibility shims/types tied to `ResourceHandle` and update affected tests/examples.
	- [ ] Validate contract-sensitive suites (`native-test`, protocol/transport contract matrix) after removal.
- [ ] Add bus-level config for refresh coordination (`fullRefreshOnly` / wait for all transports to finish).
- [ ] Centralize buffer access for bus/protocol/transport/shader layers.
	- [ ] Introduce a per-bus buffer-access object that is passed by reference instead of passing multiple `BufferHolder` instances.
	- [ ] Expose typed accessors for root pixel buffer, shader scratch buffer, protocol arena, and per-strand protocol slices.
	- [ ] Add a common buffer-access provider interface so composite busses expose the same buffer surface as single busses.
	- [ ] Consolidate protocol-buffer sizing/binding logic into one reusable binder shared by static/dynamic owning bus implementations.
	- [ ] Migrate in phases: add adapters first, switch internals/factories next, then remove legacy multi-`BufferHolder` constructor paths.




- [ ] Support non-reallocating settings alteration and expose common interfaces through composite busses.