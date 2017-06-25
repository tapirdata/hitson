import _ = require('lodash')
import { expect } from 'chai';

import wsonFactory from './wsonFactory';
import setups from './fixtures/setups';
import pairs from './fixtures/stringify-pairs';
import { saveRepr } from './fixtures/helpers'


for (const setup of setups) {
  describe(setup.name, () => {
    const wson = wsonFactory(setup.options);
    describe('parse', () => {
      for (const pair of pairs) {
        if (!_.has(pair, 's')) {
          continue
        }
        if (pair.parseFailPos != null) {
          it(`should fail to parse '${pair.s}' at ${pair.parseFailPos}`, function() {
            let e
            try {
              wson.parse(pair.s, {backrefCb: pair.backrefCb});
            } catch (e_) {
              e = e_;
            }
            expect(e.name).to.be.equal('ParseError');
            // if (typeof failPos !== 'undefined' && failPos !== null) {
            expect(e.pos).to.be.equal(pair.parseFailPos);
            // }
          })
        } else {
          it(`should parse '${pair.s}' as ${saveRepr(pair.x)}`, () => {
            expect(wson.parse(pair.s, {backrefCb: pair.backrefCb})).to.be.deep.equal(pair.x)
          })
        }
      }
    })
  })
}


