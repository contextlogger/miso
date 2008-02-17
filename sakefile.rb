# -*- ruby -*-
# sake variant sake0

# A makefile for Miso. Sake must be installed to use this makefile.
# Standard Symbian (and sdk2unix) style makefiles are also provided
# in source releases for those who do not want to install Sake.

require 'sake0/component'

# The UID has been allocated from Symbian.
$comp = Sake::Component.new(:basename => 'miso',
                            :vendor => "HIIT",
                            :version => [1, 93],
                            :uid_v8 => 0x10206ba4,
                            :caps => (%w{AllFiles} + Sake::DEV_CERT_CAPS))

if $sake_op[:kits]
  $kits = Sake::DevKits::get_exact_set($sake_op[:kits].strip.split(/,/))
else
  # Target all devices that look like they support Python builds.
  $kits = Sake::DevKits::get_all
  $kits.delete_if do |kit|
    !kit.supports_python?
  end
end

if $sake_op[:szeged]
  # use Szeged GCC 3.0 backport
  $epoc_gcc = "gcc-3.0-psion-98r2"
end

$builds = $kits.map do |kit|
  Sake::CompBuild.new :component => $comp, :devkit => kit
end

def try_load file
  begin
    load file
  rescue LoadError; end
end

# For any v9 builds, configure certificate info for signing, if you do
# want the SIS files signed as well as unsigned. You must set the
# cert_file, key_file, and passphrase properties of each relevant
# build for this.
try_load('local/signing.rb')

# We probably do not require separate documentation for every single
# build variant, as at least the interface should be just about the
# same in each file; although given that we are documenting private
# stuff, too, there could be differences, but still, let us just pick
# one version.
$doc_build = $builds.last

for build in $builds
  map = build.trait_map

  if $sake_op[:logging] and map[:has_flogger]
    map[:do_logging] = :define
  end

  list = %w{euser.lib python222.lib}
  list += %w{bluetooth.lib efsrv.lib esock.lib hal.lib sysutil.lib}
  list += %w{flogger.lib} if map[:do_logging]
  list += %w{vibractrl.lib} if map[:has_vibractrl] and not map[:has_hwrmvibra]
  list += %w{hwrmvibraclient.lib} if map[:has_hwrmvibra]
  build.libs = list

  list = %w{fs_notify_change.cpp local_epoc_py_utils.cpp module_init.cpp}
  list += %w{vibra.cpp} if map[:has_vibractrl] or map[:has_hwrmvibra]
  list = list.sort.map do |x|
    $comp.src_dir + x
  end
  build.cxx_files = list
end

task :default => [:pyd, :sis]

require 'sake0/tasks'

Sake::Tasks::def_list_devices_tasks(:builds => $builds)

Sake::Tasks::def_makefile_tasks(:builds => $builds)

Sake::Tasks::def_pyd_tasks(:builds => $builds)

Sake::Tasks::def_sis_tasks(:builds => $builds)

Sake::Tasks::def_clean_tasks(:builds => $builds)

task :all => [:makefiles, :pyd]

if $doc_build
  # C++ API documentation.
  Sake::Tasks::def_doxygen_tasks(:build => $doc_build)
  task :all => :cxxdoc

  # Python API documentation.
  py_file = $doc_build.src_dir + ($doc_build.component.basename + ".py")
  Sake::Tasks::def_pydoc_tasks(:build => $doc_build, :py_file => py_file)
  task :all => :pydoc
end

desc "Prepares web pages."
task :web do
  generated = []

  srcfiles = Dir['web/*.txt2tags.txt']
  for srcfile in srcfiles
    htmlfile = srcfile.sub(/\.txt2tags\.txt$/, ".html")
    generated.push(htmlfile)
    sh("tools/txt2tags --target xhtml --infile %s --outfile %s --encoding utf-8 --verbose" % [srcfile, htmlfile])
  end

  for htmlfile in Dir['web/*.html']
    # Tidy does not quite like the txt2tags generated docs, so
    # excluding them.
    next if generated.include? htmlfile
    sh("tidy", "-utf8", "-e", htmlfile.to_s)
  end
end

desc "Prepares downloads for the current version."
task :release do
  web_dir = $doc_build.dir + "download"
  comp = $comp
  builds = $builds

  api_sfile = comp.python_api_dir + (comp.basename + ".html")
  api_dfile = web_dir + ("%s-%s-api.html" % [comp.basename, comp.version_string])
  install api_sfile.to_s, api_dfile.to_s, :mode => 0644

  api_sfile = (comp.src_dir + (comp.basename + ".py"))
  api_dfile = web_dir + ("%s-%s-api.py" % [comp.basename, comp.version_string])
  install api_sfile.to_s, api_dfile.to_s, :mode => 0644

  unless $sake_op[:no_sis]
    for build in builds
      # For all targets we distribute and unsigned SIS, but in the case
      # of 3rd edition, we only want an unsigned SIS for DevCert
      # capability builds.
      if build.target.edition < 3 or
          build.sign_type == :dev_cert
        install build.to_proj_rel(build.long_sis_file).to_s, web_dir.to_s, :mode => 0644
      end

      # For 3rd edition builds, we distribute a self-signed SIS.
      if build.target.edition >= 3 and
          build.sign_type == :self
        install build.to_proj_rel(build.long_sisx_file).to_s, web_dir.to_s, :mode => 0644
      end
    end
  end
end

# A file in which to define uploading rules for a release.
try_load('local/uploading.rb')

Sake::Tasks::force_uncurrent_on_op_change
