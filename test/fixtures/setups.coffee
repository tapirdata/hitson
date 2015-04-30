extdefs = require './extdefs'

# connectors =
#   Point:
#     by: extdefs.Point
#     create: ->
#     split: ->

connectors =
  Point:
    name: 'Point'
    by: extdefs.Point
    precreate: () -> Object.create extdefs.Point.prototype
    postcreate: (obj, args) -> extdefs.Point.apply obj, args
    split: (p) -> p.__wsonsplit__()
  Polygon:
    name: 'Polygon'
    by: extdefs.Polygon
    create: (points) -> new extdefs.Polygon points
    hasCreate: true
    split: (p) -> p.points

module.exports = [
  {name: 'WSON', options: {connectors: connectors}}
]

