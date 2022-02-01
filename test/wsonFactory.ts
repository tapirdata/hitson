import {
  OpOptions,
  Value,
  FactoryOptions,
  Connector,
  HowNext,
  PartialCb,
  BaseStringifyError,
  BaseParseError,
} from '../src/types';
import addonFactory from '../src/';

class StringifyError extends BaseStringifyError {
  name = 'StringifierError';
  constructor(x: Value, cause: string) {
    super(x, cause);
    this.message = 'unexpected value';
  }
}

class ParseError extends BaseParseError {
  name = 'ParseError';
  constructor(s: string, pos: number, cause: string) {
    super(s, pos, cause);
    this.message = 'bad syntax';
  }
}

export interface Wson {
  escape(s: string): string;
  unescape(s: string): string;
  getTypeid(x: Value): number;
  stringify(x: Value, opt: OpOptions): string;
  parse(s: string, opt: OpOptions): Value;
  parsePartial(s: string, opt: OpOptions): Value;
  connectorOfCname(name: string): Connector<unknown>;
  connectorOfValue(value: Value): Connector<unknown>;
}

export interface Factory {
  (options: FactoryOptions): Wson;
  ParseError: typeof ParseError;
  StringifierError: typeof StringifyError;
}

const dftHowNext: HowNext = false;
const dftCb: PartialCb = () => dftHowNext;

function factory(options: FactoryOptions): Wson {
  const stringifier = new addonFactory.Stringifier(StringifyError, options);
  const parser = new addonFactory.Parser(ParseError, options);
  return {
    escape(s: string) {
      return stringifier.escape(s);
    },
    unescape(s: string) {
      return parser.unescape(s);
    },
    getTypeid(x: Value) {
      return stringifier.getTypeid(x);
    },
    stringify(x: Value, opt: OpOptions) {
      return stringifier.stringify(x, opt.haverefCb);
    },
    parse(s: string, opt: OpOptions) {
      return parser.parse(s, opt.backrefCb);
    },
    parsePartial(s: string, opt: OpOptions) {
      return parser.parsePartial(s, opt.howNext ?? dftHowNext, opt.cb ?? dftCb, opt.backrefCb);
    },
    connectorOfCname(cname: string) {
      return parser.connectorOfCname(cname);
    },
    connectorOfValue(value: Value) {
      return stringifier.connectorOfValue(value);
    },
  };
}

factory.ParseError = ParseError;
factory.StringifyError = StringifyError;

export default factory;
export { ParseError, StringifyError };
