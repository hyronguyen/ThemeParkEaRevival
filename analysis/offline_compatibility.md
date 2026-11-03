# Offline compatibility

The preview performs local file I/O and Windows rendering only. Its source contains no network client, EA SDK or mobile binary loader. It requires no emulator or account. Compiler/dependency downloads were from development-tool sources, not original game services.

| Concern | Actual static evidence | Current treatment |
|---|---|---|
| EA/Origin authentication | Account/Origin/Mayhem symbols and Android identity bridges | Not invoked or replaced in the preview; whether original guest mode bypasses all dependencies is unknown |
| Remote configuration | LoadData::parseGameSettings; Android response parser entity/economy/unlock fields | Full historical response/configuration is missing; no substitute values invented |
| Cloud/park state | LoadWorldJob, WorldInstance XML, Android fullpark routes | Original full-state and merge behavior unresolved |
| Building progression | StartBuildJob and Android build start/complete/finish routes | No simulated success responses or invented construction timers |
| Currency/purchases | ModifyCash/ModifyXP, billing/store integrations | No purchases, entitlement verification or currency-server bypass implemented |
| Local persistence | PMProfileStorage and reader/writer symbols | Evidence of some local state, not proof of complete offline gameplay; independent preview save documented separately |
| Content downloads | APK expansion loading and missing OBB | Supplied IPA assets used locally; no EA content endpoints queried |

A complete offline port needs independent local state transitions backed by recovered client rules and data. Unknown server behavior must remain marked provisional and cannot be faithfully preserved by assuming every request succeeds. The user would need to supply any missing matching expansion packages or original behavior recordings; credentials, secrets or private keys are neither needed nor requested.
