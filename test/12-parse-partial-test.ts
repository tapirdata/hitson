import { expect } from 'chai';

import { HowNext, Value } from '../src/types';
import { saveRepr } from './fixtures/helpers';
import pairs from './fixtures/partial-pairs';
import setups from './fixtures/setups';
import wsonFactory, { ParseError } from './wsonFactory';

for (const setup of setups) {
  describe(setup.name, () => {
    const wson = wsonFactory(setup.options);

    function collectPartial(s: string, nrs: HowNext[], backrefCb?: (refNum: number) => Value) {
      const result: Value[] = [];
      let nrIdx = 0;

      function cb(isValue: boolean, value: Value, pos: number) {
        result.push(isValue);
        result.push(value);
        result.push(pos);
        return nrs[nrIdx++];
      }

      wson.parsePartial(s, { howNext: nrs[nrIdx++], cb, backrefCb });

      return result;
    }

    describe('parse partial', () => {
      for (const pair of pairs) {
        const s = pair.s;
        if (s == null) {
          continue;
        }
        if (pair.failPos != null) {
          it(`should fail to parse ${saveRepr(s)} at ${pair.failPos}`, () => {
            let e;
            try {
              collectPartial(s, pair.nrs ?? [], pair.backrefCb);
            } catch (someE) {
              e = someE as ParseError;
            }
            if (e == null) {
              throw new Error('ParseError expected');
            }
            expect(e.name).to.be.equal('ParseError');
            expect(e.pos).to.be.equal(pair.failPos);
          });
        } else {
          it(`should parse ${saveRepr(s)} as ${saveRepr(pair.col)} (nrs=${saveRepr(pair.nrs)})`, () => {
            expect(collectPartial(s, pair.nrs ?? [], pair.backrefCb)).to.be.deep.equal(pair.col);
          });
        }
      }
    });
  });
}
