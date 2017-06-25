import * as extdefs from './extdefs';

let connectors = {
  Point: {
    by: extdefs.Point,
    split(p) { return p.__wsonsplit__(); },
    precreate() { return Object.create(extdefs.Point.prototype); },
    postcreate(obj, args) { obj.initialize(...args) },
  },
  Polygon: {
    by: extdefs.Polygon,
    split(p) { return p.points; },
    create(points) { return new extdefs.Polygon(points); },
    hasCreate: true
  },
  Foo: {
    by: extdefs.Foo,
    split(foo) { return [foo.y, foo.x]; },
    precreate() { return Object.create(extdefs.Foo.prototype); },
    postcreate(obj, args) { obj.y = args[0]; obj.x = args[1]; return obj},
  }
};

export default [
  {name: 'WSON', options: {connectors}}
];

