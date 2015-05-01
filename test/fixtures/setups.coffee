extdefs = require './extdefs'

connectors =
  Point:
    by: extdefs.Point
    split: (p) -> p.__wsonsplit__()
    precreate: () -> Object.create extdefs.Point.prototype
    postcreate: (obj, args) -> extdefs.Point.apply obj, args
  Polygon:
    by: extdefs.Polygon
    split: (p) -> p.points
    create: (points) -> new extdefs.Polygon points
    hasCreate: true
  Foo:
    by: extdefs.Foo
    split: (foo) -> [foo.y, foo.x]
    precreate: () -> Object.create extdefs.Foo.prototype
    postcreate: (foo, args) ->
      extdefs.Foo.call foo, args[1], args[0]

module.exports = [
  {name: 'WSON', options: {connectors: connectors}}
]

