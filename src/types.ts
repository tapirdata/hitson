export type Value = unknown;
type Arg = unknown;

export interface WsonError {
  s: string;
  pos: number;
  cause: string;
  message: string;
  name: string;
}
export type WsonErrorClass = new (s: string, pos: number, cause: string) => WsonError;

export type HowNext = boolean | [boolean, number] | null | Error;
export type PartialCb = (isText: boolean, part: string, pos: number) => HowNext;
export type BackrefCb = (idx: number) => Value;
export type HaverefCb = (x: Value) => number | null;

export interface Connector<T, A extends [Arg] = [Arg]> {
  by: T;
  split: (x: T) => A;
  create?: (...args: A) => T;
  precreate?: () => T;
  postcreate?: (x: T, args: A) => T | null | undefined;
  name?: string;
  hasCreate?: boolean;
}

export interface FactoryOptions {
  connectors?: Record<string, Connector<Value>>;
}

export interface OpOptions {
  howNext?: HowNext;
  cb?: PartialCb;
  backrefCb?: BackrefCb;
  haverefCb?: HaverefCb;
}

interface AddonStringifier {
  escape(s: string): string;
  stringify(x: Value, haverefCb?: HaverefCb | null): string;
  getTypeid(x: Value): number;
  connectorOfValue<V extends Value>(value: V): Connector<V>;
}

interface AddonParser {
  unescape(s: string): string;
  parse(s: string, backrefCb?: BackrefCb | null): Value;
  parsePartial(s: string, howNext: HowNext, cb: PartialCb, backrefCb?: BackrefCb | null): Value;
  connectorOfCname(cname: string): Connector<Value>;
}

export interface AddonFactory {
  Stringifier: new (err: WsonErrorClass, opt: FactoryOptions) => AddonStringifier;
  Parser: new (err: WsonErrorClass, opt: FactoryOptions) => AddonParser;
}
