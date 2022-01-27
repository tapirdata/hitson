export type PointArgs = [number | undefined, number | undefined];

export class Point {
  public x?: number;
  public y?: number;

  constructor(...args: PointArgs) {
    this.initialize(...args);
  }

  public initialize(x?: number, y?: number): void {
    this.x = x;
    this.y = y;
  }

  public __wsonsplit__(): PointArgs {
    return [this.x, this.y];
  }
}

export class Polygon {
  public points: Point[];

  constructor(points?: Point[]) {
    this.points = points || [];
  }
}

export type FooValue = unknown;
export type FooArgs = [FooValue, FooValue];

export class Foo {
  constructor(public x: FooValue, public y: FooValue) {}
}
