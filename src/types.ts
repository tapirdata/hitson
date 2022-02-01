export type Value = unknown;
export type AnyArgs = unknown[];
export type Class<T = Value, A extends AnyArgs = AnyArgs> = { new (...args: A): T };

export class BaseStringifyError extends Error {
  constructor(public x: Value, public cause: string) {
    super();
  }
  message = '';
  name = '';
}
export type BaseStringifyErrorClass = new (x: Value, cause: string) => BaseStringifyError;

export class BaseParseError extends Error {
  constructor(public s: string, public pos: number, public cause: string) {
    super();
  }
  message = '';
  name = '';
}
export type BaseParseErrorClass = new (s: string, pos: number, cause: string) => BaseParseError;

export type HowNext = boolean | [boolean, number] | null | Error;
export type PartialCb = (isText: boolean, part: string, pos: number) => HowNext;
export type BackrefCb = (idx: number) => Value;
export type HaverefCb = (x: Value) => number | null;

export type Splitter<T = unknown, A extends AnyArgs = AnyArgs> = (x: T) => A;
export type Creator<T = unknown, A extends AnyArgs = AnyArgs> = (args: A) => T;
export type Precreator<T = unknown> = () => T;
export type Postcreator<T = unknown, A extends AnyArgs = AnyArgs> = (x: T, args: A) => T | null | undefined;

export interface Connector<T, A extends AnyArgs = AnyArgs> {
  by: Class<T, A>;
  split: Splitter<T, A>;
  create?: Creator<T, A>;
  precreate?: Precreator<T>;
  postcreate?: Postcreator<T, A>;
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
  Stringifier: new (err: BaseStringifyErrorClass, opt: FactoryOptions) => AddonStringifier;
  Parser: new (err: BaseParseErrorClass, opt: FactoryOptions) => AddonParser;
}
