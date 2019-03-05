workflow "New workflow" {
  on = "push"
  resolves = ["Filters for GitHub Actions"]
}

action "Filters for GitHub Actions" {
  uses = "actions/bin/filter@d820d56839906464fb7a57d1b4e1741cf5183efa"
}
