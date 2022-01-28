import _ = require('lodash');
import { expect } from 'chai';

import { saveRepr } from './fixtures/helpers';
import setups from './fixtures/setups';
import pairs from './fixtures/stringify-pairs';
import wsonFactory, { ParseError } from './wsonFactory';

for (const setup of setups) {
  describe(setup.name, () => {
    const wson = wsonFactory(setup.options);
    describe('parse', () => {
      for (const pair of pairs) {
        const s = pair.s;
        if (s == null) {
          continue;
        }
        if (pair.parseFailPos != null) {
          it(`should fail to parse ${saveRepr(s)}' at ${pair.parseFailPos}`, () => {
            let e;
            try {
              wson.parse(s, { backrefCb: pair.backrefCb });
            } catch (someE) {
              e = someE as ParseError;
            }
            if (e == null) {
              throw new Error('ParseError expected');
            }
            expect(e.name).to.be.equal('ParseError');
            expect(e.pos).to.be.equal(pair.parseFailPos);
          });
        } else {
          it(`should parse ${saveRepr(s)} as ${saveRepr(pair.x)}`, () => {
            expect(wson.parse(s, { backrefCb: pair.backrefCb })).to.be.deep.equal(pair.x);
          });
        }
      }
    });
  });
}
