describe('The test harness', () => {
    it('runs the first test', () => {
        expect(2+3).toBe(5);
    });
    it('runs the second test', () => {
        expect(null).toBeFalsy();
    });

    describe('the EverythingLoaded promise', () => {
        beforeEach(async () => {
            await EverythingLoaded;
        });

        it('has access to CanvasKit', () => {
           const r = CanvasKit.LTRBRect(1, 2, 3, 4);
           expect(r.constructor.name).toEqual('Float32Array');
        });

        it('can talk to Gold', (done) => {
            expect(GoldReportPort).toBeTruthy();
            fetch(`http://localhost:${GoldReportPort}/test`)
              .then((resp) => resp.text())
              .then((text) => {
                  console.log("Response was", text);
                  expect(text).toBeTruthy();
                  done();
              }).catch((err) =>{
                  fail("Got an error talking to Gold:", err);
                  done();
            })
        })
    });
})