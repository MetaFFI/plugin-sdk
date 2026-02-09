package common

import (
	"testing"
)

func TestDetectInstalledGo(t *testing.T) {
	installed := DetectInstalledGo()
	// We expect at least one Go installation when running in CI or dev (both have Go)
	if len(installed) == 0 {
		t.Skip("No Go installation detected (optional for this test)")
		return
	}
	for i, info := range installed {
		if info.GoExe == "" {
			t.Errorf("DetectInstalledGo()[%d].GoExe is empty", i)
		}
		if info.Goroot == "" {
			t.Errorf("DetectInstalledGo()[%d].Goroot is empty", i)
		}
		if info.Version == "" {
			t.Errorf("DetectInstalledGo()[%d].Version is empty", i)
		}
		if len(info.Version) < 3 || info.Version[:2] != "go" {
			t.Errorf("DetectInstalledGo()[%d].Version should start with 'go', got %q", i, info.Version)
		}
	}
}
