// Copyright 2020 Valentin Vanelslande
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

const getRegexes = require('./common/custom-default-settings-plugin-request-regexes')
const fs = require('fs').promises

module.exports = async (github, context) => {
  const linesAfterEdit = []
  const lines = context.payload.issue.body.split('\r\n')
  const regexes = getRegexes(null, null, null)

  const uselessLines = []

  lines.forEach(line => {
    let useless = true

    for (const regex of regexes) {
      if (regex.regex.test(line)) {
        linesAfterEdit.push(line)
        useless = false
        break
      }
    }

    if (useless) {
      uselessLines.push(line)
    }
  })

  if (linesAfterEdit.length === 0) {
    await github.issues.createComment({
      issue_number: context.issue.number,
      owner: context.repo.owner,
      repo: context.repo.repo,
      body:
        'Read https://vvanelslande.github.io/vvctre/Custom-Default-Settings-Plugin-Request'
    })

    await github.issues.update({
      issue_number: context.issue.number,
      owner: context.repo.owner,
      repo: context.repo.repo,
      state: 'closed',
      labels: ['Invalid'],
      body: 'Invalid'
    })

    await github.issues.lock({
      issue_number: context.issue.number,
      owner: context.repo.owner,
      repo: context.repo.repo
    })

    await fs.writeFile('invalid', '')
  } else if (uselessLines.length > 0) {
    await github.issues.update({
      issue_number: context.issue.number,
      owner: context.repo.owner,
      repo: context.repo.repo,
      body: linesAfterEdit.join('\n')
    })

    await github.issues.createComment({
      issue_number: context.issue.number,
      owner: context.repo.owner,
      repo: context.repo.repo,
      body: `Useless lines removed:\n\`\`\`\n${uselessLines.join(
        '\n'
      )}\n\`\`\`\n\nLines that aren't in https://vvanelslande.github.io/vvctre/Custom-Default-Settings-Plugin-Request are useless lines.`
    })
    
    await fs.writeFile('edited', '')
  }
}