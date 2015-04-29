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
    split: (p) -> p.__wsonsplit__()
  Polygon:
    name: 'Polygon'
    by: extdefs.Polygon
    create: (points) -> new extdefs.Polygon points
    split: (p) -> p.points

module.exports = [
  {name: 'WSON', options: {connectors: connectors}}
]

