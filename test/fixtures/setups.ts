import { Connector, FactoryOptions } from '../../src/types';
import { Foo, FooArgs, Point, PointArgs, Polygon } from './extdefs';

// eslint-disable-next-line @typescript-eslint/no-explicit-any
const connectors: Record<string, Connector<any, any>> = {
  Point: {
    by: Point,
    split(p: Point): PointArgs {
      return p.__wsonsplit__();
    },
    precreate(): Point {
      return Object.create(Point.prototype) as Point;
    },
    postcreate(obj: Point, args: PointArgs): void {
      obj.initialize(...args);
    },
  },
  Polygon: {
    by: Polygon,
    split(p: Polygon): Point[] {
      return p.points;
    },
    create(points: Point[]): Polygon {
      return new Polygon(points);
    },
    hasCreate: true,
  },
  Foo: {
    by: Foo,
    split(foo: Foo): FooArgs {
      return [foo.y, foo.x];
    },
    precreate(): Foo {
      return Object.create(Foo.prototype) as Foo;
    },
    postcreate(obj: Foo, args: FooArgs): Foo {
      obj.y = args[0];
      obj.x = args[1];
      return obj;
    },
  },
};

const options: FactoryOptions = { connectors };
export default [{ name: 'WSON', options }];
