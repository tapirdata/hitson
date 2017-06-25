
class Point {

  public x?: number
  public y?: number

  constructor(...args) {
    this.initialize(...args)
  }

  initialize(x?: number, y?: number) {
    this.x = x;
    this.y = y;
  }
  __wsonsplit__() { return [this.x, this.y]; }
}


class Polygon {

  public points: Point[]
  
  constructor(points) {
    this.points = points || [];
  }
}

class Foo {

  public x: number
  public y: number

  constructor(x, y) {
    this.x = x;
    this.y = y;
  }
}


export { Foo, Point, Polygon }


