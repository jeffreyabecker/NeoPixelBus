[CmdletBinding()]
param(
    [string]$HubUrl = 'https://www.ledyilighting.com/addressable-pixel-ic-datasheet-hub/',
    [string]$OutputDir = 'docs/data-sheets',
    [switch]$Force,
    [switch]$ListOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$resolvedOutputDir = if ([System.IO.Path]::IsPathRooted($OutputDir)) {
    $OutputDir
}
else {
    Join-Path (Get-Location) $OutputDir
}

if (-not (Test-Path -LiteralPath $resolvedOutputDir)) {
    New-Item -ItemType Directory -Path $resolvedOutputDir -Force | Out-Null
}

Write-Host "Fetching hub page: $HubUrl"
$response = Invoke-WebRequest -Uri $HubUrl
$html = $response.Content

$sectionPattern = '(?is)<h2[^>]*>\s*(?<heading>[^<]*SPI[^<]*)\s*</h2>.*?<table[^>]*>(?<table>.*?)</table>'
$sectionMatch = [regex]::Match($html, $sectionPattern)

if (-not $sectionMatch.Success) {
    throw 'Could not find an SPI IC table on the hub page. The page structure may have changed.'
}

$spiTableHtml = $sectionMatch.Groups['table'].Value
$pdfMatches = [regex]::Matches($spiTableHtml, 'href\s*=\s*"(?<url>https?://[^"]+?\.pdf(?:\?[^"]*)?)"')

$pdfUrls = $pdfMatches |
    ForEach-Object { $_.Groups['url'].Value } |
    Sort-Object -Unique

if (-not $pdfUrls -or $pdfUrls.Count -eq 0) {
    throw 'No PDF links were found in the SPI IC table.'
}

Write-Host "Found $($pdfUrls.Count) SPI datasheet PDF link(s)."

$downloaded = 0
$skipped = 0

foreach ($url in $pdfUrls) {
    $uri = [Uri]$url
    $rawName = [System.IO.Path]::GetFileName($uri.AbsolutePath)
    if ([string]::IsNullOrWhiteSpace($rawName)) {
        Write-Warning "Skipping malformed URL: $url"
        continue
    }

    $fileName = [System.Uri]::UnescapeDataString($rawName)
    $destination = Join-Path $resolvedOutputDir $fileName

    if ($ListOnly) {
        Write-Host "SPI PDF: $url"
        continue
    }

    if ((Test-Path -LiteralPath $destination) -and (-not $Force)) {
        Write-Host "Skipping existing: $fileName"
        $skipped++
        continue
    }

    Write-Host "Downloading: $fileName"
    Invoke-WebRequest -Uri $url -OutFile $destination
    $downloaded++
}

if ($ListOnly) {
    Write-Host 'List only mode: no files downloaded.'
}
else {
    Write-Host "Done. Downloaded: $downloaded | Skipped existing: $skipped | Output: $resolvedOutputDir"
}
