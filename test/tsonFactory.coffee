'use strict'

hitson = require '../lib/'

class ParseError extends Error
  name: 'ParseError'
  constructor: (@s, @pos, cause) ->
    @message = 'bad bad syntax'



module.exports = (options) ->
  stringifier = new hitson.Stringifier()
  parser = new hitson.Parser ParseError
  escape: hitson.escape
  unescape: hitson.unescape
  stringify: (x) -> stringifier.stringify x
  parse: (x) -> parser.parse x
