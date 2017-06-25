import _ from 'lodash';
import { expect } from 'chai';

import wsonFactory from './wsonFactory';
import * as extdefs from './fixtures/extdefs';
import setups from './fixtures/setups';

for (const setup of setups) {
  describe(setup.name, () => {
    let { Point } = extdefs;
    const wson = wsonFactory(setup.options);
    it('should allow to get connector by cname', () => {
      let connector = wson.connectorOfCname('Point');
      expect(connector).to.exist;
      expect(connector.by).to.be.equal(Point);
    }
    );
    it('should allow to get connector by value', () => {
      let connector = wson.connectorOfValue(new Point());
      expect(connector).to.exist;
      expect(connector.by).to.be.equal(Point);
    })
  })
}


