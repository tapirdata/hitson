// tslint:disable:max-classes-per-file
import wson from "../lib/"

class ParseError extends Error {

  public s: string
  public pos: number
  public cause: string
  public message: string
  public name: string

  constructor(s: string, pos: number, cause: string) {
    super()
    this.s = s
    this.pos = pos
    this.cause = cause
    this.message = "bad syntax"
    this.name = "ParseError"
  }
}

class StringifierError extends Error {

  public s: string
  public pos: number
  public cause: string
  public message: string
  public name: string

  constructor(s: string, pos: number, cause: string) {
    super()
    this.s = s
    this.pos = pos
    this.cause = cause
    this.message = "bad syntax"
    this.name = "StringifierError"
  }
}

export interface Factory {
  (options: any): any
  ParseError: typeof ParseError
  StringifierError: typeof StringifierError
}

const factory = ((createOptions: any) => {
  const stringifier = new wson.Stringifier(StringifierError, createOptions)
  const parser = new wson.Parser(ParseError, createOptions)
  return {
    escape(s: string) { return stringifier.escape(s) },
    unescape(s: string) { return parser.unescape(s) },
    getTypeid(x: any) { return stringifier.getTypeid(x) },
    stringify(x: any, options: any) {
      return stringifier.stringify(x, options.haverefCb)
    },
    parse(s: string, options: any) {
      return parser.parse(s, options.backrefCb)
    },
    parsePartial(s: string, options: any) {
      return parser.parsePartial(s, options.howNext, options.cb, options.backrefCb)
    },
    connectorOfCname(cname: string) { return parser.connectorOfCname(cname) },
    connectorOfValue(value: any) { return stringifier.connectorOfValue(value) },
  }
}) as Factory

factory.ParseError = ParseError
factory.StringifierError = StringifierError

export default factory
export { ParseError, StringifierError }
