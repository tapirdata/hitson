import wson from '../lib/';

class ParseError extends Error {
  constructor(s, pos, cause) {
    super()
    this.s = s;
    this.pos = pos;
    this.cause = cause;
    this.message = 'bad syntax';
    this.name = 'ParseError';
  }
}

class StringifierError extends Error {
  constructor(s, pos, cause) {
    super()
    this.s = s;
    this.pos = pos;
    this.cause = cause;
    this.message = 'bad syntax';
    this.name = 'StringifierError';
  }
}


let factory = function(options) {
  let stringifier = new wson.Stringifier(StringifierError, options);
  let parser = new wson.Parser(ParseError, options);
  return {
    escape(s) { return stringifier.escape(s); },
    unescape(s) { return parser.unescape(s); },
    getTypeid(x) { return stringifier.getTypeid(x); },
    stringify(x, options) {
      return stringifier.stringify(x, options.haverefCb);
    },
    parse(s, options) {
      return parser.parse(s, options.backrefCb);
    },
    parsePartial(s, options) {
      return parser.parsePartial(s, options.howNext, options.cb, options.backrefCb);
    },
    connectorOfCname(cname) { return parser.connectorOfCname(cname); },
    connectorOfValue(value) { return stringifier.connectorOfValue(value); }
  };
};

factory.ParseError = ParseError;
export default factory;
