import bindings = require("bindings")

type ErrorClass = any
type HowNext = boolean | [boolean, number] | null | Error
type PartialCb = (isText: boolean, part: string, pos: number) => HowNext
type BackrefCb = (idx: number) => any
type HaverefCb = (x: any) => number | null

interface Connector<T> {
  by: T
  split: (x: T) => any[]
  create?: (...args: any[]) => T
  precreate?: () => T
  postcreate?: (x: T, args: any[]) => T | null | undefined
  name?: string
  hasCreate?: boolean
}

export interface CreateOptions {
  connectors?: Record<string, Connector<any>>
}

interface AddonStringifier {
  escape(s: string): string
  stringify(x: any, haverefCb?: HaverefCb | null): string
  getTypeid(x: any): number
  connectorOfValue(value: any): Connector<any>
}

interface AddonParser {
  unescape(s: string): string
  parse(s: string, backrefCb?: BackrefCb | null): any
  parsePartial(s: string, howNext: HowNext, cb: PartialCb, backrefCb?: BackrefCb | null): any
  connectorOfCname(cname: string): Connector<any>
}


export interface Factory {
  Stringifier: new(err: ErrorClass, opt: CreateOptions) => AddonStringifier
  Parser: new(err: ErrorClass, opt: CreateOptions) => AddonParser
}

const wsonFactory = bindings("wson_addon") as Factory

export default wsonFactory
