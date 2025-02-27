describe('Bidi Behavior', function() {
    let container;

    const assetLoadingPromises = [];

    let robotoFontBuffer = null;
    assetLoadingPromises.push(fetch('/assets/Roboto-Regular.otf').then(
        (response) => response.arrayBuffer()).then(
        (buffer) => {
            robotoFontBuffer = buffer;
        }));

    beforeEach(async () => {
        await EverythingLoaded;
        container = document.createElement('div');
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    gm('get_bidi_regions', (canvas) => {
        const sampleText = 'left1 يَهْدِيْكُمُ left2 اللَّه left3 ُ وَيُصْلِحُ left4 بَالَكُم left5 يَهْدِيْكُم';
        var bidi = CanvasKit.Bidi.Make();
        const regions = bidi.getBidiRegions(sampleText, CanvasKit.TextDirection.LTR);

        expect(regions.length).toEqual(10);
        for (var i = 0; i < 5; i ++) {
            expect(regions[i*2].level).toEqual(2);
            expect(regions[i*2 + 1].level).toEqual(1);
        }
        bidi.delete();
    });

    function reorder(bidi, input, expected) {

        const logicals = bidi.reorderVisual(input);

        var result = '[';
        for (var i = 0; i < logicals.length; ++i) {
            const logical = logicals[i];
            result += (i === 0 ? '' : ', ') + logical;
        }
        result += ']';
        expect(result).toEqual(expected);
    }

    gm('reorder_visuals', (canvas) => {
        var bidi = CanvasKit.Bidi.Make();
        reorder(bidi, [], '[]');
        reorder(bidi, [0], '[0]');
        reorder(bidi, [1], '[0]');
        reorder(bidi, [0, 1, 0, 1], '[0, 1, 2, 3]');
        bidi.delete();
    });

    function checkFlags(flags, start, end, expected) {
        for (var i = start; i < end; ++i) {
            const flag = flags[i];
            var result = '';
            result += (flag & CanvasKit.CodeUnitFlags.Ideographic.value) !== 0 ? 'I' : '';
            result += (flag & CanvasKit.CodeUnitFlags.Whitespace.value) !== 0 ? 'W' : '';
            result += (flag & CanvasKit.CodeUnitFlags.Space.value) !== 0 ? 'S' : '';
            result += (flag & CanvasKit.CodeUnitFlags.Control.value) !== 0 ? 'C' : '';
            expect(result).toEqual(expected);
        }
    }

    gm('get_code_unit_flags', (canvas) => {
        const flagsText = '   |\u{a0}\u{a0}\u{a0}|\u{0a}\u{0a}\u{0a}|満毎行';
        var codeUnits = CanvasKit.CodeUnits.Make();
        const flags = codeUnits.compute(flagsText);
        checkFlags(flags, 0, 3, 'WS');      // Whitespaces
        checkFlags(flags, 4, 7, 'S');       // Spaces (including some that are not whitespaces)
        checkFlags(flags, 8, 11, 'WSC');    // Controls
        checkFlags(flags, 12, 15, 'I');     // Ideographic
        codeUnits.delete();
    });

});
