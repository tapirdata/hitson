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
  stringify: (x, options) ->
    stringifier.stringify x, options.haverefCb
  parse: (s, options) ->
    parser.parse s, options.backrefCb
  parsePartial: (s, options) ->
    parser.parsePartial s, options.howNext, options.cb, options.backrefCb
  connectorOfCname: (cname) -> parser.connectorOfCname cname  
  connectorOfValue: (value) -> stringifier.connectorOfValue value

factory.ParseError = ParseError
module.exports = factory
