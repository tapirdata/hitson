'use strict'

wson = require '../lib/'

class ParseError extends Error
  name: 'ParseError'
  constructor: (@s, @pos, @cause) ->
    @message = 'bad syntax'

class StringifierError extends Error
  name: 'StringifierError'
  constructor: (@s, @pos, @cause) ->
    @message = 'bad syntax'


factory = (options) ->
  stringifier = new wson.Stringifier StringifierError, options
  parser = new wson.Parser ParseError, options
  escape: (s) -> stringifier.escape s
  unescape: (s) -> parser.unescape s
  stringify: (x) -> stringifier.stringify x
  parse: (s, options) ->
    parser.parse s, options.backrefCb
  parsePartial: (s, options) ->
    parser.parsePartial s, options.howNext, options.cb, options.backrefCb

factory.ParseError = ParseError
module.exports = factory
