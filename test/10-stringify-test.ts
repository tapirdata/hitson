import _ = require('lodash')
import { expect } from 'chai'
import wsonFactory from './wsonFactory'
import setups from './fixtures/setups'
import pairs from './fixtures/stringify-pairs'
import { saveRepr } from './fixtures/helpers'


for (const setup of setups) {
  describe(setup.name, () => {
    let wson = wsonFactory(setup.options);
    describe('stringify', () => {
      for (const pair of pairs) {
        if (!_.has(pair, 'x')) {
          continue
        }
        if (pair.stringifyFailPos != null) {
          it(`should fail to stringify ${saveRepr(pair.x)}`, () => {
            expect(() => wson.stringify(pair.x, {haverefCb: pair.haverefCb})).to.throw() 
          })
        } else {
          it(`should stringify ${saveRepr(pair.x)} as '${pair.s}' `, () => {
            expect(wson.stringify(pair.x, {haverefCb: pair.haverefCb})).to.be.equal(pair.s)
          })
        }
      }
    })
  })
}

