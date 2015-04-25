'use strict'

hitson = require '../lib/'

module.exports = (options) ->
  stringifier = new hitson.Stringifier()
  parser = new hitson.Parser()
  escape: hitson.escape
  unescape: hitson.unescape
  stringify: (x) -> stringifier.stringify x
  parse: (x) -> parser.parse x
