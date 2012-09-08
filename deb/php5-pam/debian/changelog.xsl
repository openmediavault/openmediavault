<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:util="urn:xslt:functions:util" xmlns:func="http://exslt.org/functions" xmlns:str="http://exslt.org/strings" extension-element-prefixes="func str">
<xsl:output method="text"/>

  <func:function name="util:maximize">
    <xsl:param name="string"/>
    <xsl:param name="line-length"/>

    <func:result>
      <xsl:variable name="tmp" select="string-length(substring-before($string,' '))"/>
      <xsl:choose>
	<xsl:when test="($tmp &gt; $line-length) or (not(contains($string, ' ')))">0</xsl:when>
        <xsl:when test="(substring($string,$line-length,1) = ' ')">
	  <xsl:value-of select="$line-length"/>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:value-of select="util:maximize(substring-after($string, ' '), $line-length - $tmp - 1) + 1 + $tmp"/>
	</xsl:otherwise>
      </xsl:choose>
    </func:result>
  </func:function>

  <func:function name="util:format">
    <xsl:param name="string"/>
    <xsl:param name="indent" select="2"/>
    <xsl:param name="line-length" select="76"/>

    <func:result>
      <xsl:choose>
        <xsl:when test="contains($string,'&#xA;') or contains($string,'&#xD;')">
          <xsl:for-each select="str:tokenize($string,'&#xA;&#xD;')">
	    <xsl:value-of select="util:format(., $indent, $line-length)"/>
          </xsl:for-each>	    
	</xsl:when>
	<xsl:when test="string-length($string) &gt; $line-length">
	  <xsl:variable name="tmp" select="util:maximize($string, $line-length)"/>
	  <xsl:value-of select="str:padding($indent,' ')"/>
	  <xsl:value-of select="substring($string, 1, $tmp)"/>
	  <xsl:text>&#xA;</xsl:text>
	  <xsl:value-of select="util:format(substring($string, $tmp + 1))"/>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:value-of select="str:padding($indent,' ')"/>
	  <xsl:value-of select="$string"/>
	  <xsl:text>&#xA;</xsl:text>
	</xsl:otherwise>
      </xsl:choose>
    </func:result>
  </func:function>

  <func:function name="util:norm">
    <xsl:param name="num"/>
    <xsl:param name="length" select="4"/>

    <xsl:choose>
      <xsl:when test="$length &gt; string-length($num)">
        <func:result select="concat('0',util:norm($num, $length - 1))"/>
      </xsl:when>
      <xsl:otherwise>
        <func:result select="$num"/>
      </xsl:otherwise>
    </xsl:choose>
  </func:function>

  <func:function name="util:extractnum">
    <xsl:param name="string"/>

    <xsl:choose>
      <xsl:when test="$string = ''">
        <func:result select="0"/>
      </xsl:when>
      <xsl:when test="$string &lt;= '9' and $string &gt;= '0'">
        <func:result select="$string"/>
      </xsl:when>
      <xsl:otherwise>
        <func:result select="util:extractnum(substring($string,1,string-length($string)-1))"/>
      </xsl:otherwise>
    </xsl:choose>
  </func:function>

  <func:function name="util:ver2num">
    <xsl:param name="version"/>

    <xsl:choose>
      <xsl:when test="contains($version,'.')">
        <func:result select="concat(util:norm(substring-before($version,'.')), util:ver2num(substring-after($version,'.')))"/>
      </xsl:when>
      <xsl:when test="$version = number($version)">
        <func:result select="concat(util:norm($version), util:norm(0))"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:variable name="tmp" select="util:extractnum($version)"/>
        <func:result select="concat(util:norm($tmp),' ', substring($version, string-length($tmp) + 1))"/>
      </xsl:otherwise>
    </xsl:choose>
  </func:function>

  <xsl:template match="package">
    <xsl:apply-templates select="release">
      <xsl:sort order="descending" select="util:ver2num(normalize-space(version))" data-type="text"/>
    </xsl:apply-templates>
    <xsl:apply-templates select="changelog/release">
      <xsl:sort order="descending" select="util:ver2num(normalize-space(version))" data-type="text"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="release">
    <xsl:text>Version </xsl:text>
    <xsl:value-of select="version"/>
    <xsl:text> - </xsl:text>
    <xsl:value-of select="date"/>
    <xsl:if test="state">
      <xsl:text> (</xsl:text>
      <xsl:value-of select="state"/>
      <xsl:text>)</xsl:text>
    </xsl:if>
    <xsl:text>&#xA;----------------------------------------&#xA;Notes:&#xA;</xsl:text>
    <xsl:value-of select="util:format(notes)"/>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>

</xsl:stylesheet>
