package common

import (
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"go.skia.org/infra/go/now"
)

func TestComputePerfJSONFileGCSPath_Success(t *testing.T) {
	fakeNow := time.Date(2022, time.January, 31, 2, 2, 3, 0, time.FixedZone("UTC+1", 60*60))
	path := computePerfJSONFileGCSPath(now.TimeTravelingContext(fakeNow), "693abc06538769c662ca1871d347323b133a5d3c", "BazelTest-Foo-Bar", "CkPp9ElAaEXyYWNHpXHU")
	assert.Equal(t, "nano-json-v1/2022/01/31/01/693abc06538769c662ca1871d347323b133a5d3c/BazelTest-Foo-Bar/results_CkPp9ElAaEXyYWNHpXHU.json", path)
}
