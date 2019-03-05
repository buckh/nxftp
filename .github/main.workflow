workflow "New workflow" {
  on = "push"
  resolves = ["GitHub Action for npm"]
}

action "Filters for GitHub Actions" {
  uses = "actions/bin/filter@d820d56839906464fb7a57d1b4e1741cf5183efa"
}

action "GitHub Action for npm" {
  uses = "actions/npm@59b64a598378f31e49cb76f27d6f3312b582f680"
  needs = ["Filters for GitHub Actions"]
}
