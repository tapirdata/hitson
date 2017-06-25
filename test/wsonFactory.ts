import wson from '../lib/';

class ParseError extends Error {

  s: string
  pos: number
  cause: string
  message: string
  name: string

  constructor(s: string, pos: number, cause: string) {
    super()
    this.s = s;
    this.pos = pos;
    this.cause = cause;
    this.message = 'bad syntax';
    this.name = 'ParseError';
  }
}

class StringifierError extends Error {

  s: string
  pos: number
  cause: string
  message: string
  name: string

  constructor(s, pos, cause) {
    super()
    this.s = s;
    this.pos = pos;
    this.cause = cause;
    this.message = 'bad syntax';
    this.name = 'StringifierError';
  }
}

export interface Factory {
  (options: any): any
  ParseError: typeof ParseError
  StringifierError: typeof StringifierError
}

const factory = ((options) => {
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
}) as Factory

factory.ParseError = ParseError
factory.StringifierError = StringifierError

export default factory;
