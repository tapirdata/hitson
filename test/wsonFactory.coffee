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
  escape: (s) -> stringifier.escape s
  unescape: (s) -> parser.unescape s
  stringify: (x) -> stringifier.stringify x
  parse: (s) -> parser.parse s
  parsePartial: (s, cb) -> parser.parsePartial s, cb
