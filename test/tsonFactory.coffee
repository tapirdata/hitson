'use strict'

tson = require '../lib/'

class ParseError extends Error
  name: 'ParseError'
  constructor: (@s, @pos, cause) ->
    @message = 'bad bad syntax'

class StringifierError extends Error
  name: 'ParseError'
  constructor: (@s, @pos, cause) ->
    @message = 'bad bad syntax'



module.exports = (options) ->
  stringifier = new tson.Stringifier StringifierError
  parser = new tson.Parser ParseError
  escape: (x) -> stringifier.escape x
  unescape: (x) -> parser.unescape x
  stringify: (x) -> stringifier.stringify x
  parse: (x) -> parser.parse x
