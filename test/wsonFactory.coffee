'use strict'

wson = require '../lib/'

class ParseError extends Error
  name: 'ParseError'
  constructor: (@s, @pos, cause) ->
    @message = 'bad bad syntax'

class StringifierError extends Error
  name: 'ParseError'
  constructor: (@s, @pos, cause) ->
    @message = 'bad bad syntax'


module.exports = (options) ->
  stringifier = new wson.Stringifier StringifierError, options
  parser = new wson.Parser ParseError, options
  escape: (x) -> stringifier.escape x
  unescape: (x) -> parser.unescape x
  stringify: (x) -> stringifier.stringify x
  parse: (x) -> parser.parse x
