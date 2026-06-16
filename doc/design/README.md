# Design docs

Architecture notes for maintainers. Each doc ends with a **Testing** section
that doubles as the spec for regression tests (invariants + the cross-repo
contract), so the "what it does" and "how we keep it from breaking" live together.

- [meteo-weather-client.md](meteo-weather-client.md) — rain/cloud-base forecast
  overlays from the meteo-map-server (`ForecastMapProvider`, layer menu).
- [sideview-route.md](sideview-route.md) — the route altitude profile side view
  and the projected wind barbs (`SideviewQuickItem`).
- [wind-field.md](wind-field.md) — the spatially/altitude/time-resolved wind field
  (`WindFieldProvider`), route integration, and the `wind.json` contract.

The three are interrelated: the weather client and the wind field both consume
the meteo-map-server; the wind field feeds both the map and the side view.
