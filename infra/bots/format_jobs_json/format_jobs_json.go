package main

/*
	Reformat the jobs.json file, sorting the jobs by name.
*/

import (
	"encoding/json"
	"os"
	"regexp"
	"sort"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_scheduler/go/specs"
)

func main() {
	b, err := os.ReadFile("infra/bots/jobs.json")
	if err != nil {
		panic(err)
	}
	b, err = updateJobsJSON(b)
	if err != nil {
		panic(err)
	}
	if err := os.WriteFile("infra/bots/jobs.json", b, 0644); err != nil {
		panic(err)
	}
}

// job represents a single entry in jobs.json.
type job struct {
	Name     string                      `json:"name"`
	CqConfig *specs.CommitQueueJobConfig `json:"cq_config"`
}

// jobSlice implements sort.Interface to sort jobs by name.
type jobSlice ([]*job)

func (s jobSlice) Less(i, j int) bool {
	return s[i].Name < s[j].Name
}

func (s jobSlice) Len() int {
	return len(s)
}

func (s jobSlice) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

func updateJobsJSON(oldJobsContents []byte) ([]byte, error) {
	var jobs []*job
	if err := json.Unmarshal(oldJobsContents, &jobs); err != nil {
		return nil, skerr.Wrapf(err, "failed to decode jobs.json")
	}

	sort.Sort(jobSlice(jobs))

	newJobsContents, err := json.MarshalIndent(jobs, "", "  ")
	if err != nil {
		return nil, skerr.Wrapf(err, "failed to encode jobs.json")
	}

	// Replace instances of `"cq_config": null`; these are cluttery and
	// unnecessary, but we can't use omitempty because that causes the Marshaler
	// to also omit `"cq_config": {}` which indicates that a job *should* be on
	// the CQ.  We also omit some whitespace to keep things relatively compact.
	jobsJSONReplaceRegex := regexp.MustCompile(`(?m)\{\n\s*"name":\s*"(\S+)",\n\s*"cq_config":\s*null\n\s*}`)
	jobsJSONReplaceContents := []byte(`{"name": "$1"}`)
	return jobsJSONReplaceRegex.ReplaceAll(newJobsContents, jobsJSONReplaceContents), nil
}
