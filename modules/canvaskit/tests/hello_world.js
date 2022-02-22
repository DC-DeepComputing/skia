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

        it('can talk to Gold once', (done) => {
            const payload = {
                name: "test_001",
                b64_data: btoa("This could have been a PNG_" + (new Date().toLocaleString()))
            }

            fetch('/gold_rpc/test', {
                body: JSON.stringify(payload),
                method: 'POST',
            }).then((resp) => {
                  expect(resp.status).toEqual(201); // StatusCreated
                  done();
            }).catch((err) =>{
                  fail('Got an error talking to Gold:', err);
                  done();
            })
        });

        it('can talk to Gold twice', (done) => {
            const payload = {
                name: "test_002",
                b64_data: btoa("This is some other data " + (new Date().toLocaleString()))
            }

            fetch('/gold_rpc/test', {
                body: JSON.stringify(payload),
                method: 'POST',
            }).then((resp) => {
                expect(resp.status).toEqual(201); // StatusCreated
                done();
            }).catch((err) =>{
                fail('Got an error talking to Gold:', err);
                done();
            })
        });
    });
})